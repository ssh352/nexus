import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import definitionsService from 'services/definitions';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.traderRowHeight = 27;
    this.visibleHeight = 0;
  }

  /** @private */
  onExpandClick(event) {
    let $contentSlideWrapper = $(event.currentTarget).parent().parent().find('.content-slide-wrapper');
    let numTraders = this.componentModel.accounts.traders.length;
    let contentHeight = this.traderRowHeight * numTraders;
    let isExpanded = !$('#' + this.componentModel.componentId).find('.icon-expand').hasClass('expanded');
    if (isExpanded) {
      $contentSlideWrapper.stop().animate({
        height: contentHeight
      });
      $(event.currentTarget).removeClass('collapsed').addClass('expanded');
      $('#' + this.componentModel.componentId).find('.trader-row.non-match').animate({
        height: this.traderRowHeight
      });
    } else {
      $contentSlideWrapper.stop().animate({
        height: this.visibleHeight
      });
      $(event.currentTarget).removeClass('expanded').addClass('collapsed');
      $('#' + this.componentModel.componentId).find('.trader-row.non-match').animate({
        height: 0
      });
    }
  }

  setContentSlideWrapperToVisibleHeight() {
    let $contentSlideWrapper = $('#' + this.componentModel.componentId).find('.content-slide-wrapper');
    $contentSlideWrapper.height(this.visibleHeight);
  }

  initialize() {
    this.setContentSlideWrapperToVisibleHeight();
  }

  componentDidUpdate() {
    this.setContentSlideWrapperToVisibleHeight();
    let $component = $('#' + this.componentModel.componentId);
    $component.find('.trader-row').removeAttr('style');
    $component.find('.icon-expand').removeClass('collapsed').removeClass('expanded');
    if ($component.find('.trader-row.match').size() >= 1 ||
      isGroupNameMatch.apply(this, [this.componentModel.searchString])) {
      $component.css('display', 'inherit');
    } else {
      $component.css('display', 'none');
    }

    function isGroupNameMatch(searchString) {
      if (this.componentModel.groupName.indexOf(searchString) == 0) {
        return true;
      } else {
        return false;
      }
    }
  }

  onTraderClick(event) {
    let traderId = $(event.currentTarget).attr('data-traderId');
    traderId = parseInt(traderId);
    this.controller.navigateToAccountProfile.apply(this.controller, [traderId]);
  }

  render() {
    let componentContainerClassName = 'profile-search-panel-container';

    let groupTraders = this.componentModel.accounts.traders;
    let traders = [];
    let numVisibleTraders = 0;
    let onTraderClick = this.onTraderClick.bind(this);
    for (let i=0; i<groupTraders.length; i++) {
      let traderRowClassName = 'trader-row';
      let matchedString, nonMatchedString;
      if (groupTraders[i].name.indexOf(this.componentModel.searchString) == 0 &&
        this.componentModel.searchString !== '') {
        traderRowClassName += ' match';
        numVisibleTraders++;
        matchedString = this.componentModel.searchString;
      } else {
        traderRowClassName += ' non-match';
        matchedString = '';
      }
      nonMatchedString = groupTraders[i].name.substring(matchedString.length);

      let defaultClass = 'icon icon-';
      let traderClass = defaultClass + 'trader';
      let managerClass = defaultClass + 'manager';
      let adminClass = defaultClass + 'admin';
      let serviceClass = defaultClass + 'service';

      if (groupTraders[i].roles.isTrader) {
        traderClass += ' active';
      }
      if (groupTraders[i].roles.isManager) {
        managerClass += ' active';
      }
      if (groupTraders[i].roles.isAdmin) {
        adminClass += ' active';
      }
      if (groupTraders[i].roles.isService) {
        serviceClass += ' active';
      }

      traders.push(
        <div key={i} className={traderRowClassName}
              onClick={onTraderClick}
              data-traderId={groupTraders[i].id}>
          <span className="matched-string">{matchedString}</span>{nonMatchedString}
          <div className="roles-wrapper">
            <span title="Trader" className={traderClass}></span>
            <span title="Manager" className={managerClass}></span>
            <span title="Admin" className={adminClass}></span>
            <span title="Service" className={serviceClass}></span>
          </div>
        </div>
      );
    }
    this.visibleHeight = this.traderRowHeight * numVisibleTraders;

    let matchedTitleString, nonMatchedTitleString;
    if (this.componentModel.groupName.indexOf(this.componentModel.searchString) == 0 &&
      this.componentModel.searchString !== '') {
      matchedTitleString = this.componentModel.searchString;
    } else {
      matchedTitleString = '';
    }
    nonMatchedTitleString = this.componentModel.groupName.substring(matchedTitleString.length);

    return (
        <div id={this.componentModel.componentId} className={componentContainerClassName}>
          <div className="header">
            <span className="icon-expand" onClick={this.onExpandClick.bind(this)}></span>
            <span className="title"><span className="matched-string">{matchedTitleString}</span>{nonMatchedTitleString}</span>
          </div>
          <div className="content-slide-wrapper">
            <div className="content-wrapper">
              {traders}
            </div>
          </div>
        </div>
    );
  }
}

export default View;