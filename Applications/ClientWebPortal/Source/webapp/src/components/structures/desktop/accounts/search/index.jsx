import {Component} from 'react';
import Controller from 'components/structures/common/accounts/search/controller';
import View from './view';
import uuid from 'uuid';

class Search extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    this.controller = new Controller(componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount();
  }

  render() {
    return this.view.render();
  }
}

export default Search;