import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { Button, DisplaySize, TextField} from '../..';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** Determines if the modal is open. */
  isOpen: boolean;

  /** The error message to display. */
  errorStatus?: string;

  /** Callback to close the modal. */
  onClose?: () => void;

  /** Callback to create a new group.
   * @param name - The name of the group.
   */
  onCreateGroup?: (name: string) => void;
}

interface State {
  groupName: string,
  isSubmitted: boolean,
  isLocalError: boolean,
  localErrorMessage: string
}

/** The modal that is used to create new groups. */
export class CreateGroupModal extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    errorStatus: '',
    onClose: () => {},
    onCreateGroup: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      groupName: '',
      isSubmitted: false,
      isLocalError: false,
      localErrorMessage: ''
    };
  }

  public render(): JSX.Element {
    const modalDimensions = (() => {
      if(this.state.localErrorMessage !== '' &&
          this.props.displaySize === DisplaySize.SMALL) {
        return CreateGroupModal.MODAL_SMALL_DIMENSIONS_ERROR;
      } else if(this.state.localErrorMessage !== '' &&
          this.props.displaySize !== DisplaySize.SMALL) {
        return CreateGroupModal.MODAL_LARGE_DIMENSIONS_ERROR;
      } else if(this.state.localErrorMessage === '' &&
        this.props.displaySize == DisplaySize.SMALL) {
        return CreateGroupModal.MODAL_SMALL_DIMENSIONS;
      } else {
        return CreateGroupModal.MODAL_LARGE_DIMENSIONS;
      }
    })();
    const inputStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return CreateGroupModal.STYLE.stacked;
      } else {
        return CreateGroupModal.STYLE.linear;
      }
    })();
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return CreateGroupModal.STYLE.buttonOverride;
      }
    })();
    const errorStatus = (() => {
      console.log(this.props.displaySize, this.state.localErrorMessage);
      if(this.props.displaySize === DisplaySize.SMALL &&
          this.state.localErrorMessage) {
        return (
          <div style={CreateGroupModal.STYLE.errorMessageSmall}>
            {this.state.localErrorMessage}
          </div>);
      } else if(this.state.localErrorMessage) {
        return (
          <div style={CreateGroupModal.STYLE.errorMessage}>
            {this.state.localErrorMessage}
          </div>);
      } else {
        return null;
      }
    })();
    return(
      <OffCenterModal
          isOpen={this.props.isOpen}
          displaySize={this.props.displaySize}
          height={modalDimensions.height}
          width={modalDimensions.width}
          onClose={this.props.onClose}>
        <div style={CreateGroupModal.STYLE.wrapper}>
          <span style={CreateGroupModal.STYLE.headerWrapper}>
            <span style={CreateGroupModal.STYLE.header}>
              {CreateGroupModal.HEADER_TEXT}
            </span>
            <span>
              <img src={CreateGroupModal.IMAGE_SOURCE}
                height={CreateGroupModal.IMAGE_SIZE} 
                width={CreateGroupModal.IMAGE_SIZE}
                onClick={this.props.onClose}/>
            </span>
          </span>
          <div style={CreateGroupModal.STYLE.mediumPadding}/>
          <div style={inputStyle}>
            <TextField 
              displaySize={this.props.displaySize}
              value={this.state.groupName}
              isError={this.state.isLocalError}
              onInput={this.onNameChange}
              style={CreateGroupModal.STYLE.textInputOverride}/>
            <div style={CreateGroupModal.STYLE.inputFiller}/>
            <Button 
              label={CreateGroupModal.BUTTON_TEXT} 
              style={buttonStyle}
              onClick={this.onCreateClick}/>
          </div>
          {errorStatus}
        </div>
      </OffCenterModal>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(!prevProps.isOpen && this.props.isOpen) {
      this.setState({groupName: '', localErrorMessage: '', isLocalError: false});
    }
    if(this.state.isSubmitted && this.props.errorStatus === '') {
      this.setState({isSubmitted: false});
      this.props.onClose();
    } else if(this.state.isSubmitted && this.props.errorStatus !== '') {
      this.setState({isSubmitted: false, localErrorMessage: this.props.errorStatus});
    }
  }

  private onNameChange = (newValue: string) => {
    this.setState({groupName: newValue, isLocalError: false});
  }

  private onCreateClick = () => {
    if(this.state.groupName === '') {
      this.setState({isLocalError: true, localErrorMessage: 'Invalid input'});
      return;
    }
    this.props.onCreateGroup(this.state.groupName);
    this.setState({isSubmitted: true});
  }
  
  private static readonly STYLE = {
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    } as React.CSSProperties,
    wrapper: {
      boxSizing: 'border-box',
      padding: '18px',
      display: 'flex',
      flexDirection: 'column'
    } as React.CSSProperties,
    headerWrapper: {
      boxSizing: 'border-box' as 'border-box',
      height: '20px',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-between'
    } as React.CSSProperties,
    header: {
      font: '400 16px Roboto',
      color: '#333333'
    }as React.CSSProperties,
    mediumPadding: {
      height: '30px',
      width: '100%'
    } as React.CSSProperties,
    errorMessageSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex',
      justifyContent: 'center',
      width: '100%',
      font: '400 14px Roboto',
      color: '#E63F44',
      height: '16px',
      marginTop: '18px'
    } as React.CSSProperties,
    errorMessage: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex',
      justifyContent: 'flex-start',
      width: '100%',
      font: '400 14px Roboto',
      color: '#E63F44',
      height: '16px',
      marginTop: '18px'
    } as React.CSSProperties,
    stacked: {
      display: 'flex',
      flexDirection: 'column'
    } as React.CSSProperties,
    linear: {
      display: 'flex',
      flexDirection: 'row'
    } as React.CSSProperties,
    inputFiller: {
      height: '30px',
      width: '20px'
    }as React.CSSProperties,
    textInputOverride: {
      minWidth: '246px',
      flexGrow: 1
    } as React.CSSProperties,
    buttonOverride: {
      width: '140px'
    }as React.CSSProperties,
    clickable: {
      cursor: 'pointer' as 'pointer'
    } as React.CSSProperties
  }
  private static readonly BUTTON_TEXT = 'Create';
  private static readonly HEADER_TEXT = 'Create Group';
  private static readonly IMAGE_SIZE = '20px';
  private static readonly IMAGE_SOURCE =
    'resources/account_directory_page/create_group_modal/close.svg';
  private static readonly MODAL_SMALL_DIMENSIONS = {width: '282px', height: '184px'};
  private static readonly MODAL_LARGE_DIMENSIONS  = {width: '550px', height: '120px'};
  private static readonly MODAL_SMALL_DIMENSIONS_ERROR = {width: '282px', height: '218px'};
  private static readonly MODAL_LARGE_DIMENSIONS_ERROR  = {width: '550px', height: '154px'};
}

interface ModalProperties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The height of the modal. */
  height: string | number;

  /** The width of the modal. */
  width: string;

  /** Determines if the modal is open. */
  isOpen: boolean;

  /** Called when the modal should be closed. */
  onClose?: () => void;
}

/** This is a component that wraps a child component to style it as a modal. */
class OffCenterModal extends React.Component<ModalProperties> {
  public static readonly defaultProps = {
    onClose: () => {}
  };

  constructor(props: ModalProperties) {
    super(props);
    this.outOfBoundsClick = this.outOfBoundsClick.bind(this);
  }

  public render(): JSX.Element {
    return (
      <Transition in={this.props.isOpen}
            timeout={OffCenterModal.MENU_TRANSITION_LENGTH_MS}
            component={null}>
          {(status: string) => {
            return (
              <div style={OffCenterModal.STYLE.wrapper}
                className={css([OffCenterModal.ANIMATION.base, OffCenterModal.ANIMATION[status]])}>
                <div style={OffCenterModal.STYLE.modalWrapperLarge}
                    onClick={this.outOfBoundsClick}>
                  <div style={OffCenterModal.STYLE.topFiller} onClick={this.props.onClose}/>
                  <div style={{...OffCenterModal.STYLE.modalLarge,
                      width: this.props.width,
                      height: this.props.height}}>
                    {this.props.children}
                  </div>
                  <div style={OffCenterModal.STYLE.bottomFiller}
                    onClick={this.props.onClose}/>
                </div>
                <div style={OffCenterModal.STYLE.overlay} onClick={this.props.onClose}/>
              </div>);
          }}
      </Transition>);
  }

  private outOfBoundsClick(event: React.MouseEvent<HTMLDivElement>) {
    if(event.target === event.currentTarget) {
      this.props.onClose();
    }
  }

  private static readonly STYLE = {
    wrapper: {
      height: '100%',
      width: '100%',
      position: 'fixed' as 'fixed',
      zIndex: 9998000
    },
    topFiller: {
      flexBasis: '150px',
      flexGrow: 0,
      flexShrink: 0
    },
    bottomFiller: {
      flexBasis: '40px',
      flexGrow: 1,
      flexShrink: 0
    },
    overlay: {
      height: '100%',
      width: '100%',
      backgroundColor: '#FFFFFF',
      opacity: '0.9',
      position: 'fixed' as 'fixed',
      margin: 0,
      padding: 0,
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      zIndex: 9998000
    },
    modalWrapperLarge: {
      overflowY: 'auto' as 'auto',
      overflowX: 'hidden' as 'hidden',
      height: '100%',
      maxHeight: '100%',
      width: '100%',
      position: 'fixed' as 'fixed',
      top: 0,
      bottom: 0,
      left: 0,
      right: 0,
      margin: 0,
      padding: 0,
      display: 'flex' as 'flex',
      flexWrap: 'nowrap' as 'nowrap',
      flexDirection: 'column' as 'column',
      zIndex: 100000000
    },
    modalLarge: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 0px 6px #00000066',
      alignSelf: 'center' as 'center',
      flexGrow: 0,
      flexShrink: 0
    }
  };
  private static readonly MENU_TRANSITION_LENGTH_MS = 200;
  private static readonly ANIMATION : any = StyleSheet.create({
    base: {
      opacity: 0,
      transition:
        `opacity ${OffCenterModal.MENU_TRANSITION_LENGTH_MS}ms ease-out`
    },
    entering: {
      opacity: 1
    },
    entered: {
      opacity: 1
    },
    exited: {
      visibility: 'hidden' as 'hidden'
    }
  });
}
