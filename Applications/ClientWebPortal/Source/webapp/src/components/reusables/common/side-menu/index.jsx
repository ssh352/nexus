import {Component} from 'react';
import Controller from './controller';
import View from './view';
import uuid from 'uuid';

class SideMenu extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    this.controller = new Controller(this, componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount();
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default SideMenu;