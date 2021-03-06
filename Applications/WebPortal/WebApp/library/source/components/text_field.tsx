import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '..';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: string;

  /** Text to show if the value is empty. */
  placeholder?: string;

  /** Indicates if there is an error with the value. */
  isError?: boolean;

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the input box. */
  className?: string;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** Grants the component focus on mount. */
  autoFocus?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onInput?: (value: string) => void;
}

/** Displays a component that displays a single line of text. */
export class TextField extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: '',
    placeholder: '',
    isError: false,
    onInput: (_: string) => {}
  };

  constructor(props: Properties) {
    super(props);
    this.inputRef = React.createRef();
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TextField.STYLE.boxSmall;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return TextField.STYLE.boxMedium;
      } else {
        return TextField.STYLE.boxLarge;
      }
    })();
    const errorStyle = (() => {
      if(this.props.isError) {
        return TextField.EXTRA_STYLE.errorBox;
      } else {
        return null;
      }
    })();
    return (
      <input value={this.props.value}
        disabled={this.props.readonly}
        style={{...boxStyle, ...this.props.style}}
        placeholder={this.props.placeholder}
        ref={this.inputRef}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
          this.props.onInput(event.target.value);}}
        className={css(TextField.EXTRA_STYLE.effects, errorStyle) + ' ' +
          this.props.className}/>);
  }

  public componentDidMount() {
    if(this.props.autoFocus) {
      setTimeout(()=>{this.inputRef.current.focus();}, 0);
    }
  }

  private static readonly STYLE = {
    boxSmall: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      minWidth: '284px',
      width: '100%',
      paddingLeft: '10px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties,
    boxMedium: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      minWidth: '284px',
      paddingLeft: '10px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties,
    boxLarge: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      minWidth: '350px',
      paddingLeft: '10px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    },
    errorBox: {
      border: '1px solid #E63F44'
    },
    inputStyle: {
      flexGrow: 1
    }
  });
  private inputRef: React.RefObject<HTMLInputElement>;
}
