import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import chartColumns from './columns';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.columnsSyncCounter = 0;
  }

  /** @private */
  convertToHeaderLabel(label) {
    if (label === 'totalPnL' || label === 'Total P/L') {
      label = 'Total P/L';
    } else if (label === 'unrealizedPnL' || label === 'Unrealized P/L') {
      label = 'Unrealized P/L';
    } else if (label === 'realizedPnL' || label === 'Realized P/L') {
      label = 'Realized P/L';
    } else {
      label = this.convertToTitle.apply(this, [label]);
    }

    return label;
  }

  /** @private */
  convertToPropertyName(headerLabel) {
    let propertyName;
    if (headerLabel === 'Total P/L') {
      propertyName = 'totalPnL';
    } else if (headerLabel === 'Unrealized P/L') {
      propertyName = 'unrealizedPnL';
    } else if (headerLabel === 'Realized P/L') {
      propertyName = 'realizedPnL';
    } else {
      propertyName = this.convertToCamelCase.apply(this, [headerLabel]);
    }

    return propertyName;
  }

  /** @private */
  convertToCamelCase(str) {
    let converted = str.replace(/\W+(.)/g, function(match, chr)
    {
      return chr.toUpperCase();
    });
    return converted.charAt(0).toLowerCase() + converted.slice(1);
  }

  /** @private */
  convertToTitle(label) {
    label = label.replace(/([A-Z]+)*([A-Z][a-z])/g, "$1$2");
    label = label.charAt(0).toUpperCase() + label.slice(1);
    label = label.replace(/([A-Z])/g, ' $1').trim().replace('  ', ' ');
    return label;
  }

  initialize() {
    $('#' + this.componentModel.componentId).on('scroll', this.onScroll);

    this.onResize = this.onResize.bind(this);
    $(window).resize(this.onResize);
  }

  /** @private */
  onScroll(event) {
    let scrollTopOffset = $(this).scrollTop();
    let scrollLeftOffset = $(this).scrollLeft();
    $(this).find('.header').css('top', scrollTopOffset);
    $(this).find('.fixed-column').css('top', scrollTopOffset).css('left', scrollLeftOffset);
    let fixedHeaderHeight = $(this).find('.fixed-column-header').height();
    $(this).find('.fixed-column-body').css('top', fixedHeaderHeight - scrollTopOffset);

    let isScrollRight = event.originalEvent.wheelDeltaX < 0;
    let width = $(this).width();
    if ((scrollLeftOffset >= this.scrollWidth - width) && isScrollRight) {
      event.preventDefault();
    }
  }

  synchronizeColumnWidths() {
    let $headerTds = $('#' + this.componentModel.componentId + ' .header td');
    let $bodyTds = $('#' + this.componentModel.componentId + '>.body tr:first td');
    let $fixColumnHeaderTds = $('#' + this.componentModel.componentId + ' .fixed-column-header td');
    let $fixColumnBodyTds = $('#' + this.componentModel.componentId + ' .fixed-column-body tr:first td');
    $bodyTds.removeAttr('style');

    for (let i=0; i<$headerTds.size(); i++) {
      let headerWidth = $($headerTds[i]).outerWidth();
      let bodyWidth = $($bodyTds[i]).outerWidth();
      let maxWidth = Math.max(headerWidth, bodyWidth);
      $($headerTds[i]).css('min-width', maxWidth);
      $($bodyTds[i]).css('min-width', maxWidth);
      $($fixColumnBodyTds[i]).css('min-width', maxWidth);
      if (i == 0) {
        $($fixColumnHeaderTds[i]).css('min-width', maxWidth);
      }
    }
  }

  /** @private */
  onResize() {
    let $container = $('#' + this.componentModel.componentId);
    let $headerTable = $container.find('.header table');
    let $header = $container.find('.header');
    let headerTableWidth = $headerTable.outerWidth();
    let containerWidth = $container.outerWidth();
    if (headerTableWidth < containerWidth) {
      this.isChartTooWide = false;
      $header.removeClass('wide').addClass('wide').css('width', '100%');
      $container.find('.fixed-column-header').css('opacity', '0').css('width', '100%');
      $container.find('.fixed-column-body').css('opacity', '0');
    } else {
      this.isChartTooWide = true;
      $header.removeClass('wide').css('width', headerTableWidth).css('overflow-x', 'hidden');
      $container.find('.fixed-column-header').css('opacity', '1').width(headerTableWidth);
      $container.find('.fixed-column-body').css('opacity', '1');
    }
  }

  componentDidUpdate() {
    if (this.componentModel.data != null && this.componentModel.data[0] != null) {
      if (this.columnsSyncCounter <= 2) {
        this.synchronizeColumnWidths.apply(this);
        this.columnsSyncCounter++;
      }
    }

    let headerTableWidth = $('#' + this.componentModel.componentId + ' .header table').width();
    $('#' + this.componentModel.componentId + ' .body').css('width', headerTableWidth);
  }

  dispose() {
    $('#' + this.componentModel.componentId).unbind('scroll', this.onScroll);
    $(window).unbind('resize', this.onResize);
  }

  shouldIncludeColumn(columnHeader) {
    if (this.componentModel.filter == null ||
      columnHeader == 'Account' ||
      columnHeader == 'Security') {
      return true;
    }

    for (let i=0; i<this.componentModel.filter.length; i++) {
      if (columnHeader == this.componentModel.filter[i].name) {
        return true;
      }
    }

    return false;
  }

  /** @private */
  onHeaderClick(event) {
    let clickedColumn = event.currentTarget.innerText.trim();
    if (this.componentModel.sortingColumn == null || this.componentModel.sortingColumn.name != clickedColumn) {
      this.componentModel.sortingColumn = {
        name: clickedColumn,
        direction: 'asc'
      };
    } else if (this.componentModel.sortingColumn.direction == 'asc') {
      this.componentModel.sortingColumn.direction = 'desc';
    } else if (this.componentModel.sortingColumn.direction == 'desc') {
      this.componentModel.sortingColumn = null;
    }

    this.react.forceUpdate();
  }

  /** @private */
  sortData() {
    if (this.componentModel.sortingColumn != null) {
      this.componentModel.data.sort(getComparer.apply(this, [this.componentModel.sortingColumn]));
    }

    function getComparer(sortingColumn) {
      let propertyName = this.convertToPropertyName.apply(this, [sortingColumn.name]);

      return function(a, b) {
        if (a == null || a[propertyName] == null || b == null || b[propertyName] == null) {
          if (sortingColumn.name === 'Average Price' ||
              sortingColumn.name === 'Total P/L' ||
              sortingColumn.name === 'Unrealized P/L' ||
              sortingColumn.name === 'Cost Basis') {
            if (a != null && a[propertyName] == null && b != null && b[propertyName] != null) {
              return -1;
            } else if (a != null && a[propertyName] != null && b != null && b[propertyName] == null) {
              return 1;
            }
          }
          return 0;
        }
        let constructorName = a[propertyName].constructor.name;

        if (constructorName == 'CurrencyId') {
          let currencyA = definitionsService.getCurrencyCode(a[propertyName].value);
          let currencyB = definitionsService.getCurrencyCode(b[propertyName].value);
          if (sortingColumn.direction === 'asc') {
            return currencyA.localeCompare(currencyB);
          } else if (sortingColumn.direction === 'desc') {
            return currencyB.localeCompare(currencyA);
          }
        } else if (constructorName == 'Money') {
          if (sortingColumn.direction === 'asc') {
            return a[propertyName].compare(b[propertyName]);
          } else if (sortingColumn.direction === 'desc') {
            return b[propertyName].compare(a[propertyName]);
          }
        } else if (sortingColumn.name === 'Account') {
          if (sortingColumn.direction === 'asc') {
            return a[propertyName].name.localeCompare(b[propertyName].name);
          } else if (sortingColumn.direction === 'desc') {
            return b[propertyName].name.localeCompare(a[propertyName].name);
          }
        } else if (constructorName == 'Security') {
          let aSecurityLabel = this.getSecurityLabel.apply(this, [a[propertyName]]);
          let bSecurityLabel = this.getSecurityLabel.apply(this, [b[propertyName]]);
          if (sortingColumn.direction === 'asc') {
            return aSecurityLabel.localeCompare(bSecurityLabel);
          } else if (sortingColumn.direction === 'desc') {
            return bSecurityLabel.localeCompare(aSecurityLabel);
          }
        } else if (sortingColumn.name === 'Quantity' || sortingColumn.name === 'Cost Basis') {
          if (sortingColumn.direction === 'asc') {
            return Math.abs(a[propertyName]) - Math.abs(b[propertyName]);
          } else if (sortingColumn.direction === 'desc') {
            return Math.abs(b[propertyName]) - Math.abs(a[propertyName]);
          }
        } else if (!isNaN(a[propertyName])) {
          if (sortingColumn.direction === 'asc') {
            return a[propertyName] - b[propertyName];
          } else if (sortingColumn.direction === 'desc') {
            return b[propertyName] - a[propertyName];
          }
        } else {
          if (sortingColumn.direction === 'asc') {
            return a[propertyName].localeCompare(b[propertyName]);
          } else if (sortingColumn.direction === 'desc') {
            return b[propertyName].localeCompare(a[propertyName]);
          }
        }
      }.bind(this);
    }
  }

  /** @private */
  getSecurityLabel(security) {
    console.debug('definitions service: ');
    console.debug(definitionsService);
    let marketDatabase = definitionsService.getMarketDatabase.apply(definitionsService);
    console.debug('marketDatabase: ');
    console.debug(marketDatabase);
    return security.toString.apply(security, [marketDatabase]);
  }

  render() {
    this.sortData.apply(this);

    let columns = [];
    let rows = [];
    let accountIds = [];

    // render account column header
    let arrowIcon;
    if (this.componentModel.sortingColumn != null && this.componentModel.sortingColumn.name == 'Account') {
      let arrowIconClass;
      if (this.componentModel.sortingColumn.direction == 'asc') {
        arrowIconClass = 'icon-arrow-icon-down';
      } else if (this.componentModel.sortingColumn.direction == 'desc') {
        arrowIconClass = 'icon-arrow-icon-up';
      }
      arrowIcon = <span className={arrowIconClass}></span>;
    }
    columns.push(
      <td key={-1} className="no-select" onClick={this.onHeaderClick.bind(this)}>
        Account
        {arrowIcon}
      </td>
    );

    // render security column header
    arrowIcon = null;
    if (this.componentModel.sortingColumn != null && this.componentModel.sortingColumn.name == 'Security') {
      let arrowIconClass;
      if (this.componentModel.sortingColumn.direction == 'asc') {
        arrowIconClass = 'icon-arrow-icon-down';
      } else if (this.componentModel.sortingColumn.direction == 'desc') {
        arrowIconClass = 'icon-arrow-icon-up';
      }
      arrowIcon = <span className={arrowIconClass}></span>;
    }
    columns.push(
      <td key={-2} className="no-select" onClick={this.onHeaderClick.bind(this)}>
        Security
        {arrowIcon}
      </td>
    );

    // add column headers
    for (let i=0; i<chartColumns.length; i++) {
      let column = chartColumns[i];
      let columnHeader = this.convertToHeaderLabel.apply(this, [column.name]);
      if (this.shouldIncludeColumn.apply(this, [columnHeader])) {
        let arrowIcon;
        if (this.componentModel.sortingColumn != null && this.componentModel.sortingColumn.name == columnHeader) {
          let arrowIconClass;
          if (this.componentModel.sortingColumn.direction == 'asc') {
            arrowIconClass = 'icon-arrow-icon-down';
          } else if (this.componentModel.sortingColumn.direction == 'desc') {
            arrowIconClass = 'icon-arrow-icon-up';
          }
          arrowIcon = <span className={arrowIconClass}></span>;
        }
        columns.push(
          <td key={column.id} className="no-select" onClick={this.onHeaderClick.bind(this)}>
            {columnHeader}
            {arrowIcon}
          </td>
        );
      }
    }

    if (this.componentModel.data != null && this.componentModel.data[0] != null) {
      for (let i=0; i<this.componentModel.data.length; i++) {
        let rowData = this.componentModel.data[i];
        accountIds.push(rowData.account);
        let columns = [];
        for (let property in rowData) {
          // get the value
          let value;
          let rawAmount;
          if (property == 'account') {
            value = rowData[property].name;
          } else if (property == 'averagePrice' ||
            property == 'costBasis' ||
            property == 'fees' ||
            property == 'realizedPnL' ||
            property == 'totalPnL' ||
            property == 'unrealizedPnL') {
            if (rowData[property] == null) {
              value = 'N/A';
            } else {
              if (property == 'costBasis') {
                rawAmount = rowData[property].toString.apply(rowData[property]);
                if (rawAmount < 0) {
                  rawAmount *= -1;
                }
              } else {
                rawAmount = rowData[property].toString.apply(rowData[property]);
              }
              value = currencyFormatter.formatById(rowData.currency.toNumber(), rawAmount);
            }
          } else if (property == 'currency') {
            value = definitionsService.getCurrencyCode(rowData[property].value);
          } else if (property == 'quantity' ||
            property == 'trades' ||
            property == 'volume') {
            value = numberFormatter.formatWithComma(Math.abs(rowData[property]));
          } else if (property == 'security') {
            value = this.getSecurityLabel.apply(this, [rowData[property]]);
          } else {
            value = rowData[property];
          }

          // set classes to color the fonts for P&L values
          let className = '';
          if (property === 'totalPnL' ||
            property === 'unrealizedPnL' ||
            property === 'realizedPnL') {
            if (rawAmount > 0) {
              className = 'profit';
            } else if (rawAmount < 0) {
              className = 'loss';
            }
          }

          let columnHeader = this.convertToHeaderLabel.apply(this, [property]);
          if (this.shouldIncludeColumn.apply(this, [columnHeader])) {
            columns.push(
              <td key={i + property} className={className}>
                {value}
              </td>
            );
          }
        }

        rows.push(
          <tr key={i}>
            {columns}
          </tr>
        );
      }
    }

    return (
        <div id={this.componentModel.componentId} className="portfolio-chart-container">

          <div className="header">
            <table>
              <thead>
                <tr>
                  {columns}
                </tr>
              </thead>
            </table>
          </div>

          <div className="body">
            <table>
              <tbody>
                {rows}
              </tbody>
            </table>
          </div>
        </div>
    );
  }
}

export default View;
