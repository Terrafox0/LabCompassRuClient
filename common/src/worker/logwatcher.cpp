#include "logwatcher.h"

#ifdef Q_OS_WIN
//#include "psapi.h"
//#include "windows.h"
#endif

static const QStringList START_LINES {
    "Изаро: Поднимись из праха.",
    "Изаро: Богиня смотрит.",
    "Изаро: Справедливость восторжествует.",
    "Izaro: Ascend with precision.",
    "Izaro: The Goddess is watching.",
    "Izaro: Justice will prevail.",
};
static const QStringList FINISH_LINES {
    "Изаро: Я умираю за Империю!",
    "Изаро: Претендент, наслаждайся своей позолоченной клеткой.",
    "Изаро: Претендент, твой приз опасней самого путешествия.",
    "Изаро: Наконец-то триумф!",
    "Изаро: Тебе дарована свобода!",
    "Изаро: Ловушка тирании неизбежна.",
    "Izaro: I die for the Empire!",
    "Izaro: Delight in your gilded dungeon, ascendant.",
    "Izaro: Your destination is more dangerous than the journey, ascendant.",
    "Izaro: Triumphant at last!",
    "Izaro: You are free!",
    "Izaro: The trap of tyranny is inescapable.",
};
static const QStringList IZARO_BATTLE_START_LINES {
    "Изаро: Сложную подготовку венчает один сильный ход.",
    "Изаро: Медлительность человека на руку его врагам.",
    "Изаро: Оскверняя образы, ты оскверняешь и самого императора.",
    "Изаро: Сущность империи следует в равной мере делить на всех её жителей.",
    "Изаро: Это правитель наделяет скипетр властью, а не наоборот.",
    "Изаро: Иных спящих лучше никогда не будить.",
    "Изаро: Император хорош настолько, насколько хороши его слуги.",
    "Изаро: Когда император повелевает, мир повинуется.",
    "Izaro: Complex machinations converge to a single act of power.",
    "Izaro: Slowness lends strength to one\'s enemies.",
    "Izaro: When one defiles the effigy, one defiles the emperor.",
    "Izaro: The essence of an empire must be shared equally amongst all of its citizens.",
    "Izaro: It is the sovereign who empowers the sceptre. Not the other way round.",
    "Izaro: Some things that slumber should never be awoken.",
    "Izaro: An emperor is only as efficient as those he commands.",
    "Izaro: The emperor beckons and the world attends.",
};
static const QStringList SECTION_FINISH_LINES {
    "Изаро: Во имя богини, что за рвение!",
    "Изаро: Какое упорство!",
    "Изаро: Откуда у тебя берутся силы?",
    "Изаро: Это твоё предназначение!",
    "Izaro: By the Goddess! What ambition!",
    "Izaro: Such resilience!",
    "Izaro: You are inexhaustible!",
    "Izaro: You were born for this!",
};
static const QStringList PORTAL_SPAWN_LINES {
    ": Портал к Изаро открыт."
    ": A portal to Izaro appears."
};
static const QStringList LAB_ROOM_PREFIX { "Estate", "Domain", "Basilica", "Mansion", "Sepulchre", "Sanitorium"};
static const QStringList LAB_ROOM_SUFFIX { "Walkways", "Path", "Crossing", "Annex", "Halls", "Passage", "Enclosure", "Atrium"};
static const QRegularExpression LOG_REGEX { "^\\d+/\\d+/\\d+ \\d+:\\d+:\\d+.*?\\[.*?(\\d+)\\] (.*)$" };
static const QRegularExpression ROOM_CHANGE_REGEX { "^: <<set:\\w\\w>><<set:\\w>><<set:\\w>>Вы вошли в область (.*?)\\.$"};
//static const QRegularExpression ROOM_CHANGE_REGEX {"^: You have entered (.*?)\\.$"};

LogWatcher::LogWatcher(ApplicationModel* model)
{
    this->model = model;
    clientPath = model->get_settings()->get_poeClientPath();
    file.reset(new QFile(QDir(clientPath).filePath("logs/Client.txt")));

    timer.setInterval(1000);
    timer.setSingleShot(false);
    timer.start();
    connect(&timer, &QTimer::timeout,
        this, &LogWatcher::work);
}

void LogWatcher::work()
{
    // reset file if client path settings have changed
    auto newClientPath = model->get_settings()->get_poeClientPath();
    if (clientPath != newClientPath) {
        clientPath = newClientPath;
        file.reset(new QFile(QDir(clientPath).filePath("logs/Client.txt")));
    }

    // attempt to open file
    if (!file->isOpen()) {
        if (!file->open(QIODevice::ReadOnly)) {

            // try to detect client
//            clientPath = findGameClientPath();
              clientPath = "";
            if (clientPath.isEmpty()) {
                model->update_logFileOpen(false);
                return;
            }
            file.reset(new QFile(QDir(clientPath).filePath("logs/Client.txt")));
            if (!file->open(QIODevice::ReadOnly)) {
                model->update_logFileOpen(false);
                return;
            }
            model->get_settings()->set_poeClientPath(clientPath);
        }
        model->update_logFileOpen(true);
        file->seek(file->size());
    }

    while (true) {
        auto line = file->readLine();
        if (line.isEmpty()) {
            break;
        }
        parseLine(QString::fromUtf8(line).trimmed());
    }
}

void LogWatcher::parseLine(const QString line)
{
    auto logMatch = LOG_REGEX.match(line); // регает цифры
    if (logMatch.hasMatch()) { //если существует
        auto clientId = logMatch.captured(1); //регает айди

        auto logContent = logMatch.captured(2).trimmed(); //регает текст+ обрезает пробелы
        auto roomChangeMatch = ROOM_CHANGE_REGEX.match(logContent); //регает вход в область

        if (START_LINES.contains(logContent)) { //если в тексте есть начальный текст
            setActiveClient(clientId); //то ставит айдишник
            emit labStarted(); //и пингует старт

        } else if (roomChangeMatch.hasMatch()) { //если вход в область
            auto roomName = roomChangeMatch.captured(1); //то сохраняет имя области

if (roomName=="Аллеи усадьбы") roomName = "Estate Walkways";
if (roomName=="Аллеи имения") roomName = "Domain Walkways";
if (roomName=="Путь к усадьбе") roomName = "Estate Path";
if (roomName=="Путь к имению") roomName = "Domain Path";
if (roomName=="Перекрёсток усадьбы") roomName = "Estate Crossing";
if (roomName=="Перекрёсток имения") roomName = "Domain Crossing";
if (roomName=="Флигель храма") roomName = "Basilica Annex";
if (roomName=="Флигель особняка") roomName = "Mansion Annex";
if (roomName=="Флигель усыпальницы") roomName = "Sepulchre Annex";
if (roomName=="Флигель лазарета") roomName = "Sanitorium Annex";
if (roomName=="Залы храма") roomName = "Basilica Halls";
if (roomName=="Залы особняка") roomName = "Mansion Halls";
if (roomName=="Залы усыпальницы") roomName = "Sepulchre Halls";
if (roomName=="Залы лазарета") roomName = "Sanitorium Halls";
if (roomName=="Коридоры храма") roomName = "Basilica Passage";
if (roomName=="Коридоры особняка") roomName = "Mansion Passage";
if (roomName=="Коридоры усыпальницы") roomName = "Sepulchre Passage";
if (roomName=="Коридоры лазарета") roomName = "Sanitorium Passage";
if (roomName=="Территория усадьбы") roomName = "Estate Enclosure";
if (roomName=="Территория имения") roomName = "Domain Enclosure";
if (roomName=="Атриум храма") roomName = "Basilica Atrium";
if (roomName=="Атриум особняка") roomName = "Mansion Atrium";
if (roomName=="Атриум усыпальницы") roomName = "Sepulchre Atrium";
if (roomName=="Атриум лазарета") roomName = "Sanitorium Atrium";
if (roomName=="Испытание претендента") roomName = "Aspirant\'s Trial";
if (roomName=="Площадь претендентов") roomName = "Aspirants\' Plaza";

        auto affixes = roomName.split(' '); //и разделяет на части

              if (roomName == "Aspirants\' Plaza") {//если на площади
                setActiveClient(clientId);
                emit plazaEntered();// то пингует вход

            } else if (roomName == "Aspirant\'s Trial" || (affixes.size() == 2 && LAB_ROOM_PREFIX.contains(affixes[0]) && LAB_ROOM_SUFFIX.contains(affixes[1]))) { // если в испытании или в области
                if (isLogFromValidClient(clientId)) {
                    emit roomChanged(roomName); // то пингует изменение от имени комнаты
                }

            } else {
                if (isLogFromValidClient(clientId)) {
                    emit labExit();
                }
            }

        } else if (FINISH_LINES.contains(logContent)) {
            if (isLogFromValidClient(clientId)) {
                emit sectionFinished();
                emit labFinished();
            }

        } else if (IZARO_BATTLE_START_LINES.contains(logContent)) {
            if (isLogFromValidClient(clientId)) {
                emit izaroBattleStarted();
            }

        } else if (SECTION_FINISH_LINES.contains(logContent)) {
            if (isLogFromValidClient(clientId)) {
                emit sectionFinished();
            }

        } else if (PORTAL_SPAWN_LINES.contains(logContent)) {
            if (isLogFromValidClient(clientId)) {
                emit portalSpawned();
            }
        }
    }
}
//QString LogWatcher::findGameClientPath()
//{
//#ifdef Q_OS_WIN
//  auto hwnd = FindWindowA("POEWindowClass", nullptr);
//  if (!hwnd)
//    return QString();

//  DWORD pid;
//  GetWindowThreadProcessId(hwnd, &pid);

//  auto handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
//  if (!handle)
//    return QString();

//  char buf[1024];
//  auto r = GetModuleFileNameExA(handle, NULL, buf, 1024);
//  QString path = r ? QFileInfo(QString(buf)).dir().absolutePath() : QString();

//  CloseHandle(handle);
//  return path;
//#else
//  return QString();
//#endif
//}


void LogWatcher::setActiveClient(const QString& clientId)
{
    activeClientId = clientId;
}

bool LogWatcher::isLogFromValidClient(const QString& clientId) const
{
    return !model->get_settings()->get_multiclientSupport() || clientId == activeClientId;
}
