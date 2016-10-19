class Controller {
  constructor(react) {
    this.componentModel = cloneObject(react.props.model);
    this.onChange = react.props.onChange;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    this.componentModel = model;
    this.view.setComponentModel(this.componentModel);
  }

  onUserNotesChange(newNotes) {
    this.onChange(newNotes);
  }
}

export default Controller;