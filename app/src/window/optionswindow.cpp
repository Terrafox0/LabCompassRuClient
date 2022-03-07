#include "stdafx.h"

#include "optionswindow.h"

static const QStringList UI_SCALE_FACTORS {
    "0.5",
    "0.75",
    "1",
    "1.25",
    "1.5",
    "1.75",
    "2",
    "2.5"
};

OptionsWindow::OptionsWindow(QQmlEngine* engine, Settings* settings)
    : Window(engine, false, true)
{
    this->settings = settings;

    setSource(QUrl("qrc:/ui/options/Options.qml"));

    QStringList uiScaleFactorModel;
    std::transform(UI_SCALE_FACTORS.constBegin(), UI_SCALE_FACTORS.constEnd(), std::back_inserter(uiScaleFactorModel),
        [](const QString& s) { return s + 'x'; });
    rootObject()->findChild<QObject*>("uiScaleFactorInput")->setProperty("model", uiScaleFactorModel);

    connect(global(), SIGNAL(optionsWindowOpenChanged()),
        this, SLOT(onWindowOpenChanged()));
    connect(rootObject(), SIGNAL(openUrl(QString)),
        this, SLOT(onOpenUrl(QString)));
    connect(rootObject(), SIGNAL(browseClientPath()),
        this, SLOT(onBrowseClientPath()));
    connect(rootObject(), SIGNAL(save()),
        this, SLOT(save()));
}

void OptionsWindow::onWindowOpenChanged()
{
    bool open = global()->property("optionsWindowOpen").toBool();

    if (open) {
        load();
    }

    setVisible(open);
}

void OptionsWindow::onBrowseClientPath()
{
    const auto& file = QFileDialog::getOpenFileName(this, "Find Game Client", "", "Path of Exile Client (*.exe)");
    if (!file.isEmpty()) {
        rootObject()->setProperty("poeClientPath", QFileInfo(file).dir().absolutePath());
    }
}

void OptionsWindow::onOpenUrl(const QString& url)
{
    global()->setProperty("optionsWindowOpen", false);
    QDesktopServices::openUrl(QUrl(url));
}

void OptionsWindow::load()
{
    foreach (const auto& name, settingNames) {
        const auto& s = name.toLocal8Bit().constData();
        rootObject()->setProperty(s, settings->property(s));
    }

    int uiScaleFactorIndex = UI_SCALE_FACTORS.indexOf(settings->get_scaleFactor());
    if (uiScaleFactorIndex == -1) {
        uiScaleFactorIndex = UI_SCALE_FACTORS.indexOf("1");
    }
    rootObject()->setProperty("uiScaleFactorIndex", uiScaleFactorIndex);
}

void OptionsWindow::save()
{
    foreach (const auto& name, settingNames) {
        settings->setProperty(name.toLocal8Bit().constData(), rootObject()->property(name.toLocal8Bit().constData()));
    }

    int uiScaleFactorIndex = rootObject()->property("uiScaleFactorIndex").toInt();
    settings->setProperty("scaleFactor", UI_SCALE_FACTORS[uiScaleFactorIndex]);
}
