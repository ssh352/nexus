import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  /** Indicates an entitlement has been clicked.
   * @param entitlement - The entitlement that was clicked.
   */
  onEntitlementClick?: (entitlement: Beam.DirectoryEntry) => void;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   */
  onSubmit?: (comment: string) => void;
}

export class EntitlementsPage extends React.Component<Properties> {
  public static readonly defaultProps = {
    onEntitlementClick: () => {},
    onSubmit: () => {}
  }

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return <div/>;
  }
}
