<?php

/**
 * @file
 * Defines administration form for Paranoia module.
 */

/**
 * Administration form.
 */
function paranoia_admin_form() {
  $config = config('paranoia.settings');
  $form = array();
  $form['#config'] = 'paranoia.settings';
  $form['access_threshold'] = array(
    '#type' => 'textfield',
    '#title' => t('User access threshold (in days)'),
    '#default_value' => $config->get('access_threshold'),
    '#description' => t('Accounts that go unaccessed for more than this number
    of days can have their passwords randomized using the "drush
    paranoia-reset-stale-accounts" command.'),
  );
  $form['email_notification'] = array(
    '#type' => 'checkbox',
    '#title' => t('Email affected users'),
    '#default_value' => $config->get('email_notification'),
    '#description' => t('Check this box to notify users by email when their
    account passwords have been randomized.'),
  );

  return system_settings_form($form);
}
