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

  componentWillUpdate(model) {
    if (model != null) {
      this.componentModel.data = model.data;
      if (model.filter != null && model.filter.columns != null) {
        this.componentModel.filter = model.filter.columns;
      }
      this.view.setComponentModel(this.componentModel);
    }
  }

  /** @private */
  onFilterChange() {
    this.view.resetColumnsSyncCounter.apply(this);
  }

  componentDidMount() {
    this.view.initialize.apply(this.view);
    this.filterChangeSubId = EventBus.subscribe(Event.Portfolio.FILTER_PARAMETERS_CHANGED, this.onFilterChange.bind(this));
  }

  componentWillUnmount() {
    this.view.dispose.apply(this.view);
    EventBus.unsubscribe(this.filterChangeSubId);
  }
}

export default Controller;
