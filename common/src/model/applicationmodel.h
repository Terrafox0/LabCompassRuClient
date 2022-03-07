#ifndef APPLICATIONMODEL_H
#define APPLICATIONMODEL_H

#include "model/connectionmodel.h"
#include "model/goldendoormodel.h"
#include "model/instructionmodel.h"
#include "model/labyrinthdata.h"
#include "model/labyrinthmodel.h"
#include "model/navigationdata.h"
#include "model/plandata.h"
#include "model/plannedroutemodel.h"
#include "model/plansummarymodel.h"
#include "model/roommodel.h"
#include "model/roompresetmodel.h"
#include "settings.h"

class ApplicationModel : public QObject {
    Q_OBJECT

    QML_READONLY_UNIQUEPTR_PROPERTY(Settings, settings)

    QML_READONLY_VAR_PROPERTY(bool, newVersionAvailable)
    QML_READONLY_CSTREF_PROPERTY(QDate, currentUtcDate)
    QML_READONLY_VAR_PROPERTY(bool, logFileOpen)
    QML_READONLY_VAR_PROPERTY(bool, atPlaza)
    QML_READONLY_VAR_PROPERTY(bool, inLab)
    QML_READONLY_VAR_PROPERTY(bool, currentRoomDetermined)
    QML_READONLY_VAR_PROPERTY(bool, isValid)

    QML_READONLY_UNIQUEPTR_PROPERTY(LabyrinthModel, labyrinthModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(RoomModel, roomModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(ConnectionModel, connectionModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(GoldenDoorModel, goldenDoorModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(PlannedRouteModel, plannedRouteModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(InstructionModel, instructionModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(PlanSummaryModel, planSummaryModel)
    QML_READONLY_UNIQUEPTR_PROPERTY(RoomPresetModel, roomPresetModel)

public:
    LabyrinthData labyrinthData;
    PlanData planData;
    NavigationData navigationData;

public:
    ApplicationModel(QObject* parent = nullptr);
    bool loadFromFile(const QString& file);
    void updatePlanData(const PlanData& planData);
    void updateNavigationData(const NavigationData& navigationData);
    void updateModelOnLoadFile();
    void updateModelOnNavigation();
    void updateModelOnPresetChange();
};

#endif // APPLICATIONMODEL_H
