import { CountryDatabase, CurrencyDatabase, EntitlementDatabase,
  MarketDatabase } from '..';

/** Client used to access definitions services. */
export abstract class DefinitionsClient {

  /** Returns the organization's name. */
  public abstract get organizationName(): string;

  /** Returns the entitlement database. */
  public abstract get entitlementDatabase(): EntitlementDatabase;

  /** Returns the country database. */
  public abstract get countryDatabase(): CountryDatabase;

  /** Returns the currency database. */
  public abstract get currencyDatabase(): CurrencyDatabase;

  /** Returns the market database. */
  public abstract get marketDatabase(): MarketDatabase;

  /** Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract async open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract async close(): Promise<void>;
}
