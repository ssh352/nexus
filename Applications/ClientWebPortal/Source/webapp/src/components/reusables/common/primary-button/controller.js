class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onClick = react.props.onClick;

    this.onButtonClick = this.onButtonClick.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      overwriteMerge(this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  onButtonClick() {
    this.onClick();
  }
}

export default Controller;