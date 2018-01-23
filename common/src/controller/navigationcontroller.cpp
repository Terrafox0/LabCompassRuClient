#include "navigationcontroller.h"

NavigationController::NavigationController(ApplicationModel* model)
{
  this->model = model;
}

void NavigationController::onLabStarted()
{
  if (!model->get_isValid())
    return;

  NavigationData navigation;
  navigation.loadFromData(&model->labyrinthData, model->planData);
  navigation.updatePlannedRouteAndInstructions();
  model->updateNavigationData(navigation);
  model->update_inLab(true);
  model->get_instructionModel()->update_finishedSections(0);
}

void NavigationController::onSectionFinished()
{
  if (!model->get_isValid())
    return;

  int finishedSections = model->get_instructionModel()->get_finishedSections();
  model->get_instructionModel()->update_finishedSections(finishedSections + 1);
}

void NavigationController::onLabExit()
{
  if (!model->get_isValid())
    return;

  NavigationData navigation;
  navigation.loadFromData(&model->labyrinthData, model->planData);
  navigation.updatePlannedRouteAndInstructions();
  model->updateNavigationData(navigation);
  model->update_inLab(false);
}

void NavigationController::onRoomChanged(const QString& name)
{
  if (!model->get_isValid())
    return;

  NavigationData data = model->navigationData;
  if (data.currentRoomDetermined)
    data.previousRoom = data.currentRoom;

  // assume the user follow the planned route
  if (data.currentRoomDetermined && data.plannedRoute.size() >= 2 &&
      model->labyrinthData.getRoomFromId(data.plannedRoute[1]).name == name) {
    data.possibleCurrentRooms = {data.plannedRoute[1]};

  } else {
    QSet<QString> connectedRooms;
    foreach (const auto& current, data.possibleCurrentRooms) {
      foreach (const auto& room, model->labyrinthData.rooms)
        if (model->labyrinthData.hasConnection(current, room.id))
          connectedRooms.insert(room.id);
    }

    data.possibleCurrentRooms = QSet<QString>();
    for (auto i = connectedRooms.constBegin(); i != connectedRooms.constEnd(); i++)
      if (model->labyrinthData.getRoomFromId(*i).name == name)
        data.possibleCurrentRooms.insert(*i);
  }

  if (data.possibleCurrentRooms.size() == 1) {
    data.currentRoom = *data.possibleCurrentRooms.constBegin();
    data.currentRoomDetermined = true;
  } else {
    data.currentRoom = "";
    data.currentRoomDetermined = false;
  }

  if (data.currentRoomDetermined) {
    // unlock golden doors
    if (data.contentState.lockedDoors.contains(std::pair<QString, QString>(data.currentRoom, data.previousRoom)) ||
        data.contentState.lockedDoors.contains(std::pair<QString, QString>(data.previousRoom, data.currentRoom))) {
      data.contentState.goldenKeysInInventory -= 1;
      data.contentState.lockedDoors.removeAll(std::pair<QString, QString>(data.currentRoom, data.previousRoom));
      data.contentState.lockedDoors.removeAll(std::pair<QString, QString>(data.previousRoom, data.currentRoom));
    }

    // loot golden keys
    if (data.contentState.goldenKeyLocations.contains(data.currentRoom)) {
      data.contentState.goldenKeyLocations.removeAll(data.currentRoom);
      data.contentState.goldenKeysInInventory += 1;
    }

    // remove target
    data.targetRooms.removeAll(data.currentRoom);
  }

  data.updatePlannedRouteAndInstructions();
  model->updateNavigationData(data);
}

void NavigationController::onRoomIsTargetSet(const QString& id, bool isTarget)
{
  if (!model->get_isValid())
    return;

  if (model->get_inLab()) {
    NavigationData data = model->navigationData;
    data.targetRooms.removeAll(id);
    if (isTarget)
      data.targetRooms.append(id);
    data.updatePlannedRouteAndInstructions();
    model->updateNavigationData(data);
  } else {
    PlanData data = model->planData;
    data.targetRooms.removeAll(id);
    if (isTarget)
      data.targetRooms.append(id);
    model->updatePlanData(data);
  }
}

void NavigationController::onRoomIdSet(const QString& id)
{
  if (!model->get_isValid() || !model->get_inLab())
    return;

  NavigationData data = model->navigationData;
  data.currentRoomDetermined = true;
  data.currentRoom = id;
  data.updatePlannedRouteAndInstructions();
  model->updateNavigationData(data);
}