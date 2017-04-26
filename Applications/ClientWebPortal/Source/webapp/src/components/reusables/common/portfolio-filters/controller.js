import columns from 'components/reusables/common/portfolio-chart/columns';

class Controller {
  constructor(react, componentModel) {
    this.componentModel = componentModel;
    this.onParametersSave = react.props.onSave;
    this.isInitialized = false;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  initializeColumns() {
    this.componentModel.columns = columns;
  }

  componentWillUpdate(model) {
    if (model != null) {
      if (!this.isInitialized) {
        this.isInitialized = true;
        overwriteMerge(this.componentModel, model);
        this.componentModel.filter = {
          columns: [],
          currencies: [],
          groups: [],
          markets: []
        };
        this.initializeColumns();
      }

      this.view.setComponentModel(this.componentModel);
    }
  }

  onFilterChange(category, availableItems, selectedItems) {
    if (category === 'Group') {
      this.componentModel.groups = availableItems;
      this.componentModel.filter.groups = selectedItems;
    } else if (category === 'Currency') {
      this.componentModel.currencies = availableItems;
      this.componentModel.filter.currencies = selectedItems;
    } else if (category === 'Market') {
      this.componentModel.markets = availableItems;
      this.componentModel.filter.markets = selectedItems;
    } else if (category === 'Column') {
      this.componentModel.columns = availableItems;
      this.componentModel.filter.columns = selectedItems;
    }
    this.view.update(this.componentModel);
  }

  update() {
    this.onParametersSave(clone(this.componentModel.filter));
  }
}

export default Controller;
