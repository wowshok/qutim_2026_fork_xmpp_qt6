# qutIM → Qt 6, только XMPP

Форк: `git@github.com:wowshok/qutim_2026_fork_xmpp_qt6.git`

## Контекст

qutIM мёртв с апреля 2023 (последний коммит — `EOL qutim.org`, все сайты проекта отключены). Он не собирается на современном Qt, но причина не в том, что «Qt меняет функции»: Qt сломал совместимость дважды за 14 лет. qutIM написан под Qt 4.7/5.0 и держится за модули, которые Qt **удалил ещё в 5.6 (2016)** — QtWebKit, QtDeclarative (Qt Quick 1), QtScript, QtXmlPatterns, mac/winextras, плюс Phonon (это KDE, не Qt). То есть проект застрял на Qt ≤5.5, это 10 лет долга.

Из протоколов живы только XMPP и IRC: oscar (ICQ, закрыт в июне 2024), mrim (Mail.ru Агент, закрыт), vkontakte (API закрыт) и astral обслуживают сервисы, которых физически нет — это ~31k строк мёртвого кода.

**Нужен только XMPP.** Поэтому цель — не портировать 224k строк, а вырезать жизнеспособное ядро и довести его до Qt 6.

**Результат:** форк, который собирается и запускается на Qt 6, подключается к XMPP-серверу, показывает ростер и позволяет переписываться.

## Что остаётся

Сокращение 224k → ~100k строк, из которых большая часть — механический порт, а не переписывание.

| блок | строк | состояние |
|---|---|---|
| `src/lib/qutim` — ядро, система плагинов | 31k | почти чистое, задето в 9 файлах |
| jreen — XMPP-библиотека (вкопировать в дерево) | 30k | только QtCore/Network/Xml, 805 замен `QStringRef` |
| `src/plugins/protocols/jabber` | 18k | оставляем как есть |
| 14 плагинов `generic/` — минимальный рабочий набор | ~20k | чистые от мёртвых модулей Qt |
| `src/bin` — исполняемый файл | 2.3k | мелкие правки |

**Выкидываем ~124k строк:** протоколы oscar / mrim / vkontakte / astral / quetzal / irc; `adiumwebview` (QtWebKit); `scriptapi` (QtScript); `qmlchat`, `quickchat`, `quickcontactlist`, `kineticscroller`; все `integrations/` (mac, win, meego, dbus, indicator); `phononsound`, `yandexnarod` (QtXmlPatterns), `histman` (Qt.sql), `mobility`, `idledetector`, `mobile*`; и ~60 необязательных плагинов.

### Минимальный набор плагинов

Зависимости между плагинами **не декларативны**: сервисы находятся по типу через `Q_CLASSINFO("Service", ...)` и `ServiceManager` (`src/lib/qutim/servicemanager.cpp:40-73`). Важно: `Q_CLASSINFO("Uses", ...)` задаёт только порядок инициализации и **не гарантирует наличия провайдера** (`servicemanager.cpp:82-88` молча пропускает отсутствующий). При этом многие сервисы разыменовываются без проверки на null — то есть «необязательный по фреймворку» ≠ «необязательный на практике».

Обязательны (без них падение или тупик):

`jsonconfig` (конфиг) · `nocryptoservice` (требуется буквально по имени класса в `profilecreationpage.cpp:104`) · `qticons` · `keychain` · `password` · `authdialog` · `dataformsbackend` · `simplerosterstorage` · `comparators` · `contactmodel` · `simplecontactlist` · `adiumchat` + `tabbedchatform` + `textchat` · `xsettingsdialog` · `accountcreator` (без него **невозможно создать первый аккаунт** — `accountcreator.cpp:60-62`)

Жёсткие требования именно jabber-плагина, все с незащищённым разыменованием:
- `keychain` — `jaccount.cpp:167,272,346,422,438`, на пути подключения
- `simplerosterstorage` — `jroster.cpp:119,145,170,185,197`
- `password` — `jaccount.cpp:426-428`
- `authdialog` — `jroster.cpp:471,479,487,495`

Желательны, но не блокируют: `jsonhistory` (история переписки; без него `history.cpp:115-119` молча подставляет `NullHistory`), `localization`, `trayicon`, `kineticpopups` + `chatnotificationsbackend`, `addcontactdlg`, `contactinfo`, `joingroupchatdlg`, `metacontacts`.

## Ключевые решения

**Сборка — CMake с нуля, qbs не чиним.** Проект на qbs, причём на собственном форке `qutIM/qbs-labs`; в истории буквально «Fuck qbs. » qbs заброшен Qt Company в 2018. У jreen уже есть рабочий `CMakeLists.txt`, а на ядро + 14 плагинов CMake пишется обозримо. `src/plugins/generic/generic.qbs` — это 80 явных `references` без glob, так что вырезание подмножества в qbs тоже потребовало бы правки — выигрыша от сохранения qbs нет.

**jreen вкопировать в дерево** как `src/3rdparty/jreen`. Апстрим мёртв с января 2017, причём `HEAD` на GitHub (`c04c229d`) совпадает с пином сабмодуля — синхронизироваться не с кем, а правок будет 800+. Это убирает возню с сабмодулями и мёртвыми `git://` URL. Лицензия jreen (LGPL) это позволяет.

**Историю сохранить целиком** — 6599 коммитов, `.git` всего 27 МБ. Это форк GPL v3 проекта, атрибуция авторов важна юридически и по-человечески. Лицензионные файлы (`COPYING`, `GPL`, `CCBYSA`, `AUTHORS`) остаются; форк остаётся GPL v3+.

**jreen сначала, QXmpp потом.** jreen даёт XMPP уровня 2014: нет OMEMO, MAM, Carbons, HTTP File Upload — то есть нет сквозного шифрования, серверной истории и синхронизации между устройствами. Альтернатива — QXmpp (живая, Qt6-нативная, в Debian). Но jabber-плагин сцеплен с jreen через ~35 разных заголовков по всему коду, так что замена = переписать протокольный слой плагина. Порт jreen на Qt 6 предсказуем и механичен, даёт работающий клиент и доказывает, что ядро живо. Идти сразу в QXmpp — месяцами не видеть ничего работающего.

## Фазы

### Фаза 0. Форк

`origin` сейчас занят (`https://github.com/euroelessar/qutim.git`), поэтому вставленный скрипт GitHub в исходном виде упадёт на `git remote add origin`. И `git init` здесь не нужен — репозиторий уже есть. В git не задан `user.name`/`user.email` — коммиты не пройдут, пока не задашь.

```bash
cd /opt/qutim/qutim
git config user.name  "wowshok"
git config user.email "mrneefu@gmail.com"

git remote rename origin upstream
git remote add origin git@github.com:wowshok/qutim_2026_fork_xmpp_qt6.git
git branch -M master main
git push -u origin main          # 6599 коммитов, ~27 МБ
```

SSH к GitHub проверен — аутентификация как `wowshok` работает.

Сабмодули: все четыре репозитория (`qutIM/artwork`, `qutim-translations`, `k8json`, `qbs-labs`) **доступны**, но URL в `.gitmodules` — `git://`, который GitHub отключил в 2022. Нужен `artwork` (иконки для UI) и `k8json` (зависимость libqutim):

```bash
sed -i 's|git://github.com/|https://github.com/|g' .gitmodules
git submodule update --init artwork src/3rdparty/k8json translations
```

Затем вкопировать jreen и убрать его из сабмодулей:

```bash
git clone https://github.com/euroelessar/jreen.git /tmp/jreen
git rm --cached src/plugins/protocols/jabber/jreen
rm -rf /tmp/jreen/.git && mkdir -p src/3rdparty/jreen
cp -a /tmp/jreen/. src/3rdparty/jreen/
# из .gitmodules удалить секции jabber/jreen, vkontakte/vreen, docktile, Controls, qbs
```

Коммит: `Fork: XMPP-only Qt6 port, vendor jreen in-tree`

### Фаза 1. Вырезать мёртвый код

Отдельным коммитом, чтобы в истории было видно, что именно удалено.

```bash
git rm -r src/plugins/protocols/{oscar,mrim,vkontakte,astral,quetzal,irc}
git rm -r src/plugins/integrations
git rm -r src/plugins/generic/{adiumwebview,scriptapi,qmlchat,quickchat,quickcontactlist}
git rm -r src/plugins/generic/{kineticscroller,phononsound,yandexnarod,histman,mobility}
git rm -r src/plugins/generic/{idledetector,webhistory,sqlhistory,docktile,screenshoter}
git rm -r src/plugins/generic/{mobileabout,mobilecontactinfo,mobilenotificationssettings,mobilesettingsdialog}
git rm src/lib/qutim/scripttools.{h,cpp} src/lib/qutim/declarativeview.{h,cpp}
git rm -r qbs qutim.qbs   # и все *.qbs — заменяем на CMake
```

Остальные ~45 необязательных плагинов удалять не обязательно — достаточно не включать их в CMake. Но чем меньше дерева, тем легче ориентироваться.

### Фаза 2. jreen на Qt 6

Самый предсказуемый блок, и первый осмысленный чекпойнт: если jreen собрался — ядро тоже поедет.

jreen зависит **только** от QtCore + QtNetwork + QtXml, ни одного удалённого модуля. Внешние зависимости: GSASL (обязательна) и zlib; `3rdparty/` содержит вкопированные `jdns` и `icesupport`.

1. **`QStringRef` → `QStringView`** — 821 вхождение, но 805 из них это один и тот же шаблон параметра. Это интерфейс XML-парсера, реализованный ~40 классами-фабриками:
   ```cpp
   bool canParse(const QStringRef &name, const QStringRef &uri, const QXmlStreamAttributes &attributes);
   void handleStartElement(const QStringRef &name, const QStringRef &uri, const QXmlStreamAttributes &attributes);
   void handleEndElement(const QStringRef &name, const QStringRef &uri);
   void handleCharacterData(const QStringRef &text);
   ```
   В Qt 6 `QXmlStreamReader::name()/namespaceUri()/text()` возвращают `QStringView`. Замена:
   ```bash
   cd src/3rdparty/jreen
   grep -rl 'QStringRef' src | xargs sed -i 's/const QStringRef *&/QStringView /g; s/const QStringRef&/QStringView /g'
   ```
   Остаётся ~26 мест вручную: локальные переменные (`QStringRef t` и подобные) и 10 явных конструирований `QStringRef(...)`. Самая высокая концентрация — `mucroomfactory.cpp`, `mucroomfactory_p.h`, `dataformfactory.cpp`, `vcardfactory.cpp`.

2. **Мелочь:** 1 × `QTextCodec` → `QStringConverter`, 1 × `qSort` → `std::sort`. `QRegExp` и `QLinkedList` — ноль вхождений.

3. **CMakeLists на Qt 6.** Сейчас там только `find_package(Qt5Core/Qt5Network)` и ветка Qt4. Заменить на `find_package(Qt6 REQUIRED COMPONENTS Core Network Xml)`, перейти на `qt_add_library` / `CMAKE_AUTOMOC`.

4. **GSASL.** Используется в двух файлах (`saslfeature.cpp`, `saslfeature_p.h`), всего 12 функций — все они живы в GSASL 2.x, так что переход с 1.x на `libgsasl7-dev` 2.2 должен пройти без правок. Если всплывёт — правка локальна.

**Чекпойнт:** `libjreen` собирается и линкуется под Qt 6.

### Фаза 3. Ядро на Qt 6

Ядро (31k строк) задето мёртвым Qt всего в нескольких местах — это день-два работы.

**Блокер, который касается всего проекта:** QtScript прописан в зависимостях **каждого** плагина и самого ядра — `src/plugins/Plugin.qbs:63` и `src/lib/libqutim.qbs:18`:
```
Depends { name: "Qt"; submodules: [ "core", "gui", "network", "script", "widgets", "quick" ] }
```
В CMake это просто не переносим. В коде:
- `src/lib/qutim/message.{h,cpp}` — убрать `Message::scriptRegister(QScriptEngine*)` (`message.h:93`, `message.cpp:29-31,82-103`); это меняет публичный ABI, но библиотеку всё равно пересобираем целиком
- `src/bin/src/widgets/modulemanagerimpl.cpp:32-33` — `#include <QScriptEngine>/<QScriptValue>` не используется, удалить
- `scripttools.{h,cpp}` удалены в фазе 1 (их потреблял только мёртвый `scriptapi`)

Точечные правки:
- `dataforms.{h,cpp}` — 10 × `QRegExp` → `QRegularExpression`. **Внимание:** это валидация XMPP data forms, нужна jabber'у (регистрация, vCard, поиск, adhoc) — не упрощать.
- `utils.cpp:448`, `tooltip.cpp:142`, `profile.cpp:199` — `QRegExp` → `QRegularExpression`
- `utils.cpp:33`, `statisticshelper.cpp:37` — `QDesktopWidget` → `QScreen` / `QGuiApplication::screenAt()`
- `modulemanager.cpp:181` — `endl` → `Qt::endl`; `:299` — убрать `qRegisterMetaTypeStreamOperators` (в Qt 6 автоматически); `:1273` и `modulemanagerimpl.cpp:106,109` — `QRegExp::Wildcard` → `QRegularExpression::wildcardToRegularExpression()`
- `src/bin/src/main.cpp:47` — `qsrand` → `QRandomGenerator`
- `setMargin` → `setContentsMargins` (33 вхождения по проекту)

Система плагинов переносится без изменений: уже на `QPluginLoader` + `Q_PLUGIN_METADATA` (`modulemanager.cpp:426-574`).

**Важно про верификацию плагинов:** `modulemanager.cpp` сверяет `QUTIM_VERSION_STRING` в плагине с версией ядра **точным сравнением**, так что ядро и все плагины обязаны собираться одной версией — частичная пересборка не работает.

**Чекпойнт:** `libqutim.so` собирается под Qt 6.

### Фаза 4. jabber + плагины

1. CMake для `src/bin` и 14 плагинов минимального набора.
2. `keychain` — внешняя зависимость QtKeychain, в Debian есть `qtkeychain-qt6-dev` 0.13.2. В `keychain.qbs:5` зашито `cpp.dynamicLibraries: ["qt5keychain"]` → в CMake линковать `qt6keychain`.
3. `qticons` — его QML-страница настроек (`qml/settings/qticons/main.qml`) использует `QtQuick.Controls 1.1`, удалённый в Qt 6. Сама иконочная логика — чистый C++ (`QIcon::fromTheme`), так что QML-группу просто не устанавливать.
4. jabber-плагин правок почти не требует — он общается с jreen, а не с Qt напрямую. Jingle/VoIP оставить выключенным (`property bool jingle: false`).

**Чекпойнт:** `qutim` запускается, показывает мастер создания аккаунта.

### Фаза 5. Проверка

Сборка на твоей машине; на VPS X11 forwarding уже работает (`DISPLAY=localhost:10.0`), плюс установлен `Xvfb` для headless-проверок.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
./build/bin/qutim
```

Сквозной сценарий:
1. Запуск → мастер создания профиля (нужны `jsonconfig` + `nocryptoservice`)
2. Создание XMPP-аккаунта (`accountcreator`), пароль (`password` → `keychain`)
3. Подключение к серверу, статус online
4. Ростер подгружается и отображается (`simplerosterstorage` → `contactmodel` → `simplecontactlist` → `comparators`)
5. Двойной клик по контакту → окно чата (`adiumchat` + `tabbedchatform` + `textchat`)
6. Отправка и приём сообщения
7. Запрос авторизации от нового контакта (`authdialog`)
8. Перезапуск → аккаунт и история на месте (`jsonhistory`)

Падение на любом шаге, скорее всего, означает отсутствующий провайдер сервиса — ищи незащищённый `ServicePointer<...>->` по списку выше.

### Фаза 6 (опционально). Современный XMPP

Только после того, как всё работает. `libqxmpp-dev` в Debian 12 — версия 1.4.0, а **OMEMO появился в QXmpp 1.5**, так что для шифрования понадобится более новая версия из бэкпортов или сборка из исходников. Переезжать постепенно, за интерфейсами jabber-плагина: Carbons и MAM (мультидевайс и серверная история) дают больше всего пользы на единицу работы, OMEMO — самый трудоёмкий.

## Риски

**Сборка на VPS опасна для твоих сервисов.** Там 961 МБ RAM (свободно ~400 МБ, swap 2.1 ГБ) и **1 ядро**, при этом работают mariadbd, php-fpm ×4, nginx (`/var/www` 770 МБ), x-ui/xray и openvpn. g++ на тяжёлом Qt-файле берёт 400 МБ – 1 ГБ. Риск не в том, что сборка упадёт, а в том, что **OOM-killer выберет жертвой базу, а не компилятор**. Если всё же собирать там — ограничивать: `systemd-run --scope -p MemoryMax=500M ninja -j1`, и `-O1 -g0`. Диск: свободно 1.0 ГБ из 9.9 ГБ, но безопасно освобождается ~1.05 ГБ (`/root/.npm` 579 МБ, `/root/.cache/puppeteer` 259 МБ, `/var/log` 140 МБ, `/var/cache/apt` 78 МБ). Пакеты сборки — 122 МБ (69 пакетов). Поскольку собираешь на другой машине, это всё справочно.

**Остальное:**
- `QStringRef` → `QStringView` — не полностью эквивалентная замена: `QStringView` не владеет данными. Если где-то значение переживает свой `QXmlStreamReader`, будет use-after-free. Проверить ~26 мест ручной правки особенно внимательно.
- CMake с нуля на 14 плагинов + кодогенерация. В qbs есть `Rule`, который генерирует `<name>genplugin.cpp` из `*.plugin.json` + `plugintemplate.cpp` (`src/plugins/Plugin.qbs`) — это надо воспроизвести в CMake, иначе плагины не зарегистрируются.
- Точное сравнение `QUTIM_VERSION_STRING` означает, что любая несогласованность версий даёт молча не загружающиеся плагины — при отладке «плагин не виден» проверять это первым.
- `artwork` — отдельный репозиторий с иконками; без него UI будет без графики.

## Порядок коммитов

1. `Fork: XMPP-only Qt6 port, vendor jreen in-tree` (фаза 0)
2. `Drop dead protocols, Qt Quick 1, WebKit and QtScript plugins` (фаза 1)
3. `jreen: port to Qt 6 (QStringRef -> QStringView, CMake)` (фаза 2)
4. `libqutim: port to Qt 6, drop QtScript bindings` (фаза 3)
5. `Build minimal XMPP plugin set with CMake` (фаза 4)
