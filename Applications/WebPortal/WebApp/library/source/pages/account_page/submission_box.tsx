import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';
import {CommentBox} from './comment_box';
import {SubmitButton} from './submit_button';

interface Properties {

  /** Determines how a form gets submitted depending on the account's roles. */
  roles: Nexus.AccountRoles;

  /** The status message to display. */
  status?: string;

  /** Indicates the form should be submitted. */
  onClick?: () => void;
}

/** Displays the components needed to submit an account related form. */
export class SubmissionBox extends React.Component<Properties> {
  public static defaultProps = {
    onClick: () => {}
  };

  /** @return The comment currently typed in the CommentBox. */
  public getComment(): string {
    if(this.commentBox) {
      return this.commentBox.getComment();
    }
    return '';
  }

  public render(): JSX.Element {
    const commentBox = (() => {
      if(!this.props.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <CommentBox ref={(ref) => this.commentBox = ref}/>;
      }
    })();
    const commentBoxPadding = (() => {
      if(!this.props.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <Padding size='30px'/>;
      }
    })();
    const message = (() => {
      switch(this.props.status) {
        case 'Submitted':
        case 'Saved':
          return <span className={css([SubmissionBox.MESSAGE_STYLE.base,
            SubmissionBox.MESSAGE_STYLE.valid])}>Submitted</span>;
        default:
          return <span className={css([SubmissionBox.MESSAGE_STYLE.base,
            SubmissionBox.MESSAGE_STYLE.invalid])}>Server issue</span>;
      }
    })();
    return (
      <VBoxLayout width='100%'>
        {commentBox}
        {commentBoxPadding}
        <HBoxLayout width='100%'>
          <Padding size='calc(50% - 123px)'/>
          <SubmitButton isDisabled={false}
            roles={this.props.roles}
            onClick={this.props.onClick}/>
          <Padding size='calc(50% - 123px)'/>
        </HBoxLayout>
        <Padding size='18px'/>
        <HBoxLayout width='100%'>
          <Padding/>
          {this.props.status}
          <Padding/>
        </HBoxLayout>
      </VBoxLayout>);
  }

  private static MESSAGE_STYLE = StyleSheet.create({
    base: {
      font: '400 14px Roboto',
      width: '100%',
      textAlign: 'center'
    },
    valid: {
      color: '#36BB55'
    },
    invalid: {
      color: '#E63F44'
    }
  });
  private commentBox: CommentBox;
}