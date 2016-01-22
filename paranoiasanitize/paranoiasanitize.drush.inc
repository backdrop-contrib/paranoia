<?php

/**
 * @file
 * Drush integration for the paranoiasanitize module.
 */

/**
 * Implements hook_drush_command().
 */
function paranoiasanitize_drush_command() {
  return array(
    'paranoia-sql-sanitize-whitelist' => array(
      'description' => dt('Sanitizes (deletes or munges) data by default. Can be configured to do less.'),
    ),
    'paranoia-build-sanitize-whitelist' => array(
      'description' => dt('Helps you build an initial whitelist. If it provides advice for a contrib, please post that as an issue on drupal.org.'),
    ),
  );
}

/**
 * Writes baseline sql sanitize whitelist hooks based on current db contents.
 */
function drush_paranoiasanitize_paranoia_build_sanitize_whitelist() {
  drush_log(dt('This command will build hooks based on current db contents. If a table is empty then it will suggest it be truncated as part of the paranoiasanitizing process. If you want to, you can truncate some tables and then there will be less manual adjusting of the output of this command.'), 'status');
  if (!drush_confirm(dt('Are you ready to continue?'))) {
    drush_die('Exiting');
  }

  // Get a list of tables that exist in the db.
  $existing_tables = db_query('SHOW TABLES;')->fetchAllKeyed(0, 0);

  // Get a list of tables that Drupal knows about.
  $original_schema = drupal_get_complete_schema();
  global $databases;
  // Fix prefixes.
  $schema = array();
  foreach ($original_schema as $table_name => $data) {
    $schema[_paranoiasanitize_apply_db_prefix($table_name, $databases)] = $data;
  }

  // Group all tables by module.
  $tables_by_module = array();
  foreach ($existing_tables as $table) {
    if (array_key_exists($table, $schema)) {
      $tables_by_module[$schema[$table]['module']][] = $table;
    }
    else {
      $tables_by_module['_tables_not_in_schema'][] = $table;
    }
  }
  ksort($tables_by_module);

  // Now write some hooks.
  $whitelist_hooks = '';
  foreach ($tables_by_module as $module => $table_list) {
    if (function_exists($module . '_paranoiasanitize_sql_sanitize_operations')) {
      drush_log(dt("A function exists for '@module', review the settings and consider if you want to override them.", array('@module' => $module)), 'warning');
      continue;
    }
    if (!empty($whitelist_hooks)) {
      $whitelist_hooks .= PHP_EOL . '  );' . PHP_EOL . '}' . PHP_EOL;
    }
    $whitelist_hooks .= PHP_EOL . "/**
* Implements hook_paranoiasanitize_sql_sanitize_operations().
*/
function {$module}_paranoiasanitize_sql_sanitize_operations() {" . PHP_EOL;
    $whitelist_hooks .= '  return array(';
    foreach ($table_list as $table) {
      $count = db_query("SELECT COUNT(1) FROM " . $table)->fetchField();
      $columns = implode("', '", _paranoiasanitize_get_columns($table));
      // If the table is currently empty, suggest it be made empty.
      if (0 == $count) {
        $whitelist_hooks .= "
    '$table' => array(
      'queries' => array('TRUNCATE {$table}'),
      'fields' => array('$columns'),
    ),";
      }
      // If it has data, suggest leaving alone. The site builder should review
      // and change this to fix it, ideally.
      else {
        $whitelist_hooks .= PHP_EOL . "    '$table' => array(
      'queries' => array(''),
      'fields' => array('$columns'),
    ),";
      }
    }
  }

  // If we created any output, close it off properly and print it.
  if (!empty($whitelist_hooks)) {
    $whitelist_hooks .= PHP_EOL . '  );' . PHP_EOL . '}' . PHP_EOL;
    echo PHP_EOL . $whitelist_hooks . PHP_EOL;
    if (array_key_exists('_tables_not_in_schema', $tables_by_module)) {
      drush_log(dt("Your site has @count table(s) that are not associated with a hook_schema from a module currently enabled on the site. These tables have been added to a hook '_tables_not_in_schema_paranoiasanitize_sql_sanitize_operations'. You should rename that function to match some real module on your site so that function will actually get called.",
        array('@count' => count($tables_by_module['_tables_not_in_schema']))), 'warning');
    }
    drush_log(dt('You can copy and paste that blob into a site-specific module or, ideally, help contrib authors by contributing this hook to their module.'), 'success');
  }
  else {
    drush_log(dt('All the tables in your db have a hook defined. You should now confirm they are actually good.'), 'success');
  }
}

/**
 * Returns an array of columns in the database for a given table.
 *
 * @param string $table_name
 *   The table name, with any prefix.
 *
 * @return array
 *   Array of columns and some data like the default value.
 */
function _paranoiasanitize_get_columns($table_name) {
  $columns = array();
  $table_info = db_query("SHOW COLUMNS FROM $table_name")->fetchAll();
  foreach ($table_info as $column_info) {
    $columns[] = $column_info->Field;
  }
  return $columns;
}

/**
 * Callback for `drush paranoia-sql-sanitize-whitelist`.
 */
function drush_paranoiasanitize_paranoia_sql_sanitize_whitelist() {
  drush_log(dt('You started sanitizing your db in a manner that is really, really destructive.'), 'warning');
  drush_log(dt('If you have multiple Drupal sites storing tables in a single database and you use prefixes to separate them then this tool cannot handle that and will delete a bunch of stuff you probably want to keep. Also, kinda crazy architecture you have there, just fyi.'), 'warning');
  if (!drush_confirm(dt('Do you want to continue?'))) {
    drush_die('Aborting.');
  }

  // Get a list of tables that exist in the db and their columns.
  $existing_tables = db_query('SHOW TABLES;')->fetchAllKeyed(0, 0);
  foreach ($existing_tables as $table_name) {
    $existing_tables[$table_name] = array('fields' => _paranoiasanitize_get_columns($table_name));
  }

  // Get a list of what to do with tables from hooks.
  global $databases;
  $whitelist_tables = array();
  // Like module_invoke_all, but respects weights & only stores top result.
  $hook = 'paranoiasanitize_sql_sanitize_operations';
  foreach (module_implements($hook) as $module) {
    $function = $module . '_' . $hook;
    if (function_exists($function)) {
      $result = call_user_func_array($function, array());
      foreach ($result as $table_name => $actions) {
        $table_name = _paranoiasanitize_apply_db_prefix($table_name, $databases);
        // TODO: it could be valuable to try to merge together values if not
        // all columns are covered. Perhaps two implementations together get
        // to a valid set of queries.
        if (!array_key_exists($table_name, $whitelist_tables)) {
          // Also modify that list to be aware of prefixes.
          $whitelist_tables[$table_name] = $actions;
        }
        else {
          watchdog('paranoiasanitize', 'Skipping @module data for @table as a prior hook provided this data', array(
            '@module' => $module,
            '@table' => $table_name,
          ), WATCHDOG_WARNING);
        }
      }
    }
  }

  // Compare whitelist and potential changes and determine what to do.
  $derived_changes = _paranoiasanitize_decide_what_to_do($existing_tables, $whitelist_tables);

  drush_log(dt('Welp, there is the list of changes that will be made.'), 'warning');
  if (!drush_confirm(dt('Do you want to continue?'))) {
    drush_die('Aborting.');
  }

  // Do the stuff in the list.
  foreach ($derived_changes as $table_name => $data) {
    foreach ($data['queries'] as $query) {
      if (!empty($query)) {
        db_query($query);
      }
    }
  }
}
