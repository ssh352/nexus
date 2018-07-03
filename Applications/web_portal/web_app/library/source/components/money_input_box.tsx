import * as Nexus from 'nexus';
import * as React from 'react';

/** Specifies the React properties used to render a MoneyInputBox. */
interface Properties {

  /** The minimum allowed value (inclusive). */
  min?: Nexus.Money;

  /** The maximum allowed value (inclusive). */
  max?: Nexus.Money;

  /** The initial value to display. */
  value?: Nexus.Money;

  /** The event handler for when a change is made. */
  onChange?: (value?: Nexus.Money) => (boolean | void);
}

interface State {
  value: Nexus.Money;
}

/** Displays an input box for money values. */
export class MoneyInputBox extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      value: props.value || Nexus.Money.ZERO
    }
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onWheel = this.onWheel.bind(this);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const inputStyle = {
      boxSizing: 'border-box',
      width: '100%',
      height: '34px',
      font: '16px Roboto',
      border: '1px solid #C8C8C8',
      paddingLeft: '10px',
      paddingRight: '10px'
    } as any;
    return (
      <div>
        <input style={inputStyle} type='text'
          ref={(input) => { this._input = input; }}
          value={this.state.value.toString()}
          onKeyDown={this.onKeyDown} onWheel={this.onWheel}
          onChange={this.onChange}/>
      </div>);
  }

  public componentDidUpdate() {
    if(this._start != null) {
      this._input.setSelectionRange(this._start, this._end);
      this._start = null;
      this._end = null;
    }
  }

  private onKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if(event.keyCode === 38) {
      this.increment();
    } else if(event.keyCode === 40) {
      this.decrement();
    }
  }

  private onWheel(event: React.WheelEvent<HTMLInputElement>) {
    if(document.activeElement !== event.target) {
      return;
    }
    if(event.deltaY > 0) {
      this.decrement();
    } else if(event.deltaY < 0) {
      this.increment();
    }
  }

  private onChange(event: React.ChangeEvent<HTMLInputElement>) {
    const pointIndex = event.target.value.search('\\.');
    const sanitizedValue = (() => {
      if(pointIndex === -1) {
        const leftPortion = event.target.value.substr(0,
          this._input.selectionStart - 1);
        const insertPortion = event.target.value.substr(
          this._input.selectionEnd - 1, 1);
        let rightPortion = event.target.value.substr(
          this._input.selectionEnd);
        rightPortion = rightPortion.padEnd(2, '0');
        return leftPortion + insertPortion + '.' + rightPortion;
      }
      const secondPointIndex = event.target.value.substr(pointIndex + 1).search(
        '\\.');
      if(secondPointIndex === -1) {
        return event.target.value;
      } else if(this._input.selectionStart > pointIndex + 1) {
        return '';
      } else if(secondPointIndex === 0) {
        return event.target.value.slice(0, pointIndex + 1) +
          event.target.value.slice(pointIndex + 2);
      } else {
        return event.target.value.slice(0, pointIndex + 1) + '00';
      }
    })();
    let value = Nexus.Money.parse(sanitizedValue);
    if(value === null) {
      this._start = this._input.selectionStart - 1;
      this._end = this._input.selectionEnd - 1;
      this.forceUpdate();
      return;
    }
    if(this.props.min && value.compare(this.props.min) < 0 ||
        this.props.max && value.compare(this.props.max) > 0) {
      return;
    }
    this.update(value);
  }

  private increment() {
    const increment = this.state.value.add(Nexus.Money.ONE);
    if(this.props.max && increment.compare(this.props.max) > 0) {
      return;
    }
    this.update(increment);
  }

  private decrement() {
    const decrement = this.state.value.subtract(Nexus.Money.ONE);
    if(this.props.min && decrement.compare(this.props.min) < 0) {
      return;
    }
    this.update(decrement);
  }

  private update(value: Nexus.Money) {
    if(this.props.onChange) {
      const commit = this.props.onChange(value);
      if(commit != null && commit === false) {
        return;
      }
    }
    this._start = this._input.selectionStart;
    this._end = this._input.selectionEnd;
    this.setState({
      value: value
    });
  }

  private _input: HTMLInputElement;
  private _start: number;
  private _end: number;
}