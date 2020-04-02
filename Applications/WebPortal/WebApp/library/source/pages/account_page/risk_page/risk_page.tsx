import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine, PageWrapper } from '../../..';
import { SubmissionInput } from '..';
import { RiskParametersView } from './risk_parameters_view';

interface Properties {

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The type of display to render on. */
  displaySize: DisplaySize;

  /** The risk parameters to display. */
  parameters: Nexus.RiskParameters;

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   * @param parameters - The parameters to submit.
   */
  onSubmit?: (comment: string, parameters: Nexus.RiskParameters) => void;
}

interface State {
  comment: string;
  parameters: Nexus.RiskParameters;
}

/** Displays a risk page. */
export class RiskPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    status: '',
    onSubmit: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      comment: '',
      parameters: this.props.parameters
    }
    this.onCommentChange = this.onCommentChange.bind(this);
    this.onParametersChange = this.onParametersChange.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
  }

  render() {
    const containerStyle= (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return RiskPage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return RiskPage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return RiskPage.STYLE.contentLarge;
      }
    })();
    const lineWrapperStyle = (() => {
      if(this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return RiskPage.STYLE.lineWrapperAdmin;
      } else {
        return RiskPage.STYLE.lineWrapper;
      }
    })();
    return(
      <PageWrapper>
        <div style={containerStyle}>
          <RiskParametersView
            parameters={this.state.parameters}
            currencyDatabase={this.props.currencyDatabase}
            displaySize={this.props.displaySize}
            onChange={this.onParametersChange}/>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <div style={lineWrapperStyle}>
            <HLine color={RiskPage.LINE_COLOR}/>
          </div>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <SubmissionInput comment={this.state.comment}
            roles={this.props.roles} isError={this.props.isError}
            status={this.props.status} 
            isEnabled
            onChange={this.onCommentChange}
            onSubmit={this.onSubmit}/>
        </div>
      </PageWrapper>);
  }

  private onCommentChange(comment: string) {
    this.setState({comment: comment});
  }

  private onParametersChange(parameters: Nexus.RiskParameters) {
    this.setState({parameters: parameters});
  }

  private onSubmit() {
    this.props.onSubmit(this.state.comment, this.state.parameters);
  }

  private static readonly STYLE = {
    contentSmall: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box' as 'border-box',
      minWidth: '284px',
      flexShrink: 1,
      flexGrow: 1,
      maxWidth: '424px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    },
    contentMedium: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box' as 'border-box',
      width: '768px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    },
    contentLarge: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box' as 'border-box',
      width: '1036px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    lineWrapperAdmin: {
      width: '246px'
    },
    lineWrapper: {
      width: '100%'
    }
  }
  private static readonly LINE_COLOR = '#E6E6E6';
}
