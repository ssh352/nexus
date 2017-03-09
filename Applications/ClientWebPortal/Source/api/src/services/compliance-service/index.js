import httpConnectionManager from '../commons/http-connection-manager';

/** Spire compliance service client class */
class ComplianceService {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Compliance Service Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  loadComplianceRuleEntries(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/load_directory_entry_compliance_rule_entry';
    let payload = {
      directory_entry: directoryEntry.toData()
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .catch(this.logErrorAndThrow);
  }

  addComplianceRuleEntry(directoryEntry, state, schema) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/add_compliance_rule_entry';
    let payload = {
      directory_entry: directoryEntry.toData(),
      state: state,
      schema: schema
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  updateComplianceRuleEntry(ruleEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/update_compliance_rule_entry';
    let payload = {
      rule_entry: ruleEntry
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  deleteComplianceRuleEntry(id) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'compliance_service/update_compliance_rule_entry';
    let payload = {
      id: id
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }
}

export default ComplianceService;
