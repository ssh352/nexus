import adminClient from 'utils/spire-clients/admin';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';

class Controller {
  constructor(componentModel) {
    this.componentModel = cloneObject(componentModel);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountEntitlements = adminClient.loadAccountEntitlements.apply(
      adminClient,
      [directoryEntry]
    );
    let loadAccountRoles = adminClient.loadAccountRoles.apply(adminClient, [directoryEntry]);

    return Promise.all([
      loadAccountEntitlements,
      loadAccountRoles
    ]);
  }

  /** @private */
  getGroupEntry(id) {
    for (let i=0; i<this.componentModel.entitlements.length; i++) {
      let groupEntry = this.componentModel.entitlements[i].group_entry;
      if (groupEntry.id === id) {
        return groupEntry;
      }
    }
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.accountEntitlements = responses[0];
      this.componentModel.entitlements = definitionsService.getEntitlements.apply(definitionsService);
      this.componentModel.directoryEntry = directoryEntry;
      this.componentModel.roles = responses[1];
      this.componentModel.userName = directoryEntry.name;
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = cloneObject(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  onEntitlementSelected(id) {
    let groupEntry = this.getGroupEntry(id);
    this.componentModel.accountEntitlements.push(groupEntry);
    this.view.update(this.componentModel);
  }

  onEntitlementDeselected(id) {
    for (let i=0; i<this.componentModel.accountEntitlements.length; i++) {
      if (this.componentModel.accountEntitlements[i].id === id) {
        this.componentModel.accountEntitlements.splice(i, 1);
      }
    }
    this.view.update(this.componentModel);
  }

  save() {
    let directoryEntry = this.componentModel.directoryEntry;
    adminClient.storeAccountEntitlements(directoryEntry, this.componentModel.accountEntitlements)
      .then(this.view.showSaveSuccessMessage)
      .catch(this.view.showSaveFailMessage);
  }
}

export default Controller;