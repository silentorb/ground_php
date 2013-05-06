<?php

class Songbird_Email implements Bird {
  public function sing($song) {
    foreach ($song['listeners'] as $account) {
      $this->send_email($song, $account);
    }
  }

  public function send_email($song, $account) {
//    if (!$this->does_receive_emails())
//      return;
//    if (!$this->user)
//      $this->user = user_load($this->uid);

    if (isset($account->mail)) {
      $email = $account->mail;
    }
    else {
      $email = $account->mail;
    }

    $theme_data['customer'] = $this;
    watchdog('songbird', "Emailing $template to " . $email . '.');
    $body = theme($template, $theme_data);
    if (!$body) {
      watchdog(loyalist, "Email $template was not found.");
    }
    else {
      mimemail('no-reply@loyalist.us', $email, $subject, $body, false);
      watchdog('loyalist', "Email $template sent to " . $email . '.');
    }
  }

}
