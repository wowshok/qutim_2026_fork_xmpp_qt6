
'use strict';

// Adium styles scroll through document.body.scrollTop, as WebKit had it. Chromium
// scrolls the document element in standards mode: body.scrollTop always reads 0 and
// writes to it are ignored, so the styles never scrolled to new messages
(function() {
    var scrollTop = Object.getOwnPropertyDescriptor(Element.prototype, 'scrollTop');
    Object.defineProperty(HTMLBodyElement.prototype, 'scrollTop', {
        configurable: true,
        get: function() {
            var root = document.scrollingElement;
            return root && root !== this ? root.scrollTop : scrollTop.get.call(this);
        },
        set: function(value) {
            var root = document.scrollingElement;
            if (root && root !== this)
                root.scrollTop = value;
            else
                scrollTop.set.call(this, value);
        }
    });
})();

var client = (function() {
    var client = {};
    var webChannel = undefined;
    var connection = undefined;
    var pendingCalls = [];

    var methods = [
        'debugLog',
        'appendNick',
        'contextMenu',
        'appendText',
    ];

    function createDefaultCallback(name) {
        return function() {
            var originalArgs = Array.prototype.slice.call(arguments);
            pendingCalls.push(function() {
                client[name].apply(undefined, originalArgs);
            });
        }
    }

    for (var i = 0; i < methods.length; ++i) {
        var method = methods[i];
        client[method] = createDefaultCallback(method);
    }

    client.zoomImage = function(element) {
        var fullClass = 'fullSizeImage';
        var scaledClass = 'scaledToFitImage';

        if (element.classList.contains(fullClass)) {
            element.classList.add(scaledClass);
            element.classList.remove(fullClass);
        } else if (element.classList.contains(scaledClass)) {
            element.classList.add(fullClass);
            element.classList.remove(scaledClass);
        } else {
            return false;
        }

        alignChat(true);
        return true;
    };

    client.setDefaultFont = function(family, size) {
        var body = document.body;
        body.style.fontFamily = family;
        body.style.fontSize = size + 'px';
    };

    client.markMessage = function(id, success) {
        var element = document.querySelector('#message' + id);
        if (element) {
            if (success) {
                element.classList.remove('notDelivered');
                element.classList.add('delivered');
            } else {
                element.classList.add('failedToDeliver');
            }
        }
    };

    client.setTopic = function() {
        var element = document.getElementById('topicEdit');
        var topic = element ? element.innerText : '';
        if (connection)
            connection.setTopic(topic);
        else
            pendingCalls.push(function() { connection.setTopic(topic); });
    };

    client.updateTopic = function(html) {
        var element = document.getElementById('topic');
        if (element)
            element.innerHTML = html;
    };

    client.clearFocus = function() {
        var elements = document.querySelectorAll('.focus');
        for (var i = 0; i < elements.length; ++i)
            elements[i].classList.remove('focus', 'firstFocus');
    };

    // Called by qutIM once qwebchannel.js is loaded, the C++ side is published as "client"
    client.initWebChannel = function() {
        // Pages of the same profile that are not chats (the inspector) have no transport
        if (typeof qt === 'undefined' || !qt.webChannelTransport)
            return;
        webChannel = new QWebChannel(qt.webChannelTransport, function(channel) {
            connection = channel.objects.client;

            function createRealCallback(name) {
                return function() {
                    connection[name].apply(connection, arguments);
                };
            }

            for (var i = 0; i < methods.length; ++i) {
                var method = methods[i];
                client[method] = createRealCallback(method);
            }

            var pending = pendingCalls;
            pendingCalls = [];

            for (i = 0; i < pending.length; ++i)
                pending[i]();
        });
    };

    // Keep the chat scrolled to the bottom while the user has not scrolled up:
    // the page is built before the view gets its size, and images (url previews,
    // emoticons) grow the messages after the style has decided about scrolling
    var stickToBottom = true;

    function isAtBottom() {
        var root = document.scrollingElement || document.documentElement;
        return root.scrollHeight - window.innerHeight - window.scrollY < 10;
    }

    function keepAtBottom() {
        if (stickToBottom)
            window.scrollTo(0, (document.scrollingElement || document.documentElement).scrollHeight);
    }

    window.addEventListener('scroll', function() {
        stickToBottom = isAtBottom();
    });
    window.addEventListener('resize', keepAtBottom);
    document.addEventListener('DOMContentLoaded', function() {
        if (window.ResizeObserver)
            new ResizeObserver(keepAtBottom).observe(document.body);
    });

    return client;
})();
