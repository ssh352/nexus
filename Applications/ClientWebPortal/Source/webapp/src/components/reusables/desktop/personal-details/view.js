import './style.scss';
import React from 'react';
import moment from 'moment';
import CommonView from 'components/reusables/common/personal-details/common-view';
import definitionsService from 'services/definitions';

class DesktopView extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let defaultClass = 'role icon icon-';
    let traderClass = defaultClass + 'trader';
    let managerClass = defaultClass + 'manager';
    let adminClass = defaultClass + 'admin';
    let serviceClass = defaultClass + 'service';

    if (this.componentModel.roles.isTrader) {
      traderClass += ' active';
    }
    if (this.componentModel.roles.isManager) {
      managerClass += ' active';
    }
    if (this.componentModel.roles.isAdmin) {
      adminClass += ' active';
    }
    if (this.componentModel.roles.isService) {
      serviceClass += ' active';
    }

    let registrationDate = moment(this.componentModel.registrationTime, moment.ISO_8601).toDate().toLocaleDateString();

    let details;
    if (this.componentModel.isReadOnly) {
      let countryName = definitionsService.getCountryName.apply(definitionsService, [this.componentModel.country]);

      details =
        <div className="details-container">
          <div className="personal-detail-row">
            <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                   readOnly/>
          </div>

          <div className="personal-detail-row">{this.componentModel.userName}</div>

          <div className="personal-detail-row">
            <span title="Trader" className={traderClass}/>
            <span title="Manager" className={managerClass}/>
            <span title="Admin" className={adminClass}/>
            <span title="Service" className={serviceClass}/>
          </div>

          <div className="personal-detail-row">{registrationDate}</div>

          <div className="personal-detail-row">{this.componentModel.id}</div>

          <div className="personal-detail-row">
            <input className="email-input" type="text" defaultValue={this.componentModel.email}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <textarea defaultValue="" rows="1" className="address-input" readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="city-input" type="text" defaultValue={this.componentModel.city}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="province-input" type="text" defaultValue={this.componentModel.province}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="country-input" type="text" defaultValue={countryName} readOnly/>
          </div>

        </div>
    } else {
      let countryOptions = [];
      let countries = definitionsService.getCountries();
      for (let i=0; i<countries.length; i++) {
        let country = countries[i];
        countryOptions.push(
          <option key={i} value={country.code}>{country.name}</option>
        );
      }

      details =
        <div className="details-container">
          <div className="personal-detail-row editable">
            <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row">{this.componentModel.userName}</div>

          <div className="personal-detail-row">
            <span title="Trader" className={traderClass}/>
            <span title="Manager" className={managerClass}/>
            <span title="Admin" className={adminClass}/>
            <span title="Service" className={serviceClass}/>
          </div>

          <div className="personal-detail-row">{registrationDate}</div>

          <div className="personal-detail-row">{this.componentModel.id}</div>

          <div className="personal-detail-row editable">
            <input className="email-input" type="text" defaultValue={this.componentModel.email}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <textarea defaultValue="" rows="1" className="address-input"/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <input className="city-input" type="text" defaultValue={this.componentModel.city}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <input className="province-input" type="text" defaultValue={this.componentModel.province}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <select className="country-input" defaultValue={this.componentModel.country} onChange={this.onChange.bind(this)}>
              {countryOptions}
            </select>
          </div>
        </div>
    }

    return (
        <div id={this.componentModel.componentId} className="personal-details-container">
          <div className="labels-container">
            <div className="personal-label-row">First Name</div>
            <div className="personal-label-row">Last Name</div>
            <div className="personal-label-row">Username</div>
            <div className="personal-label-row">Role(s)</div>
            <div className="personal-label-row">Registration Date</div>
            <div className="personal-label-row">ID</div>
            <div className="personal-label-row">E-mail</div>
            <div className="personal-label-row address">Address</div>
            <div className="personal-label-row">City</div>
            <div className="personal-label-row">Province/State</div>
            <div className="personal-label-row">Country</div>
          </div>
          {details}
        </div>
    );
  }
}

export default DesktopView;
