<?php

class Dynamic_Query {

  protected $objects = array();
  public $main_table = 'node';

  public function __construct($main_table) {
    $this->main_table = $main_table;
  }

  function add_filter($clause) {
    $this->filters[] = $clause;
  }

  function add_field($clause) {
    $this->fields[] = $clause;
  }

  function add_join($clause) {
    $this->joins[] = $clause;
  }

  function add_post($clause) {
    $this->post_clauses[] = $clause;
  }

  function generate_pager($offset = 0, $limit = 0) {
    if ($offset == 0) {
      if ($limit == 0)
        return '';
      else
        return " () LIMIT $limit";
    }
    else {
      if ($limit == 0) {
        $limit = 18446744073709551615;
      }

      return " LIMIT $offset, $limit";
    }
  }

  function add_pager() {
    $this->limit = $this->generate_pager((int) $_GET['offset'], (int) $_GET_['limit']);
  }

  function paged_sql($sql) {
    if ($this->limit != '')
      $sql .= ' ' . $this->limit;

    return $sql;
  }

  function remove_field($table, $field_name) {
    if ($this->objects[$table])
      unset($this->objects[$table]->fields[$field_name]);
  }

  function generate_sql() {
    global $user;

    $sql = 'SELECT ';

    if (count($this->fields) > 0) {
      $sql .= ', ';
    }
    $sql .= implode(', ', $this->fields);

    foreach ($this->objects as $object) {
      $sql .= $object->render_fields();
    }

    $sql .= ' FROM ' . $this->main_table . ' ';
    $sql .= ' ' . implode(' ', $this->joins);

    if (count($this->filters) > 0) {
      $sql .= ' WHERE ' . implode(' AND ', $this->filters);
    }
    $sql .= ' ' . implode(' ', $this->post_clauses);

    return $sql;
  }

  function run($return_sql = false) {
    $result = new stdClass();
    $result->nodes = array();
    $sql = $this->generate_sql();
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);
    $rows = db_query($paged_sql);
    while ($row = db_fetch_object($rows)) {
      $this->process_row($row);
      $result->nodes[] = $row;
    }

    $this->post_process_result($result);

    if ($return_sql)
      $result->sql = $paged_sql;

    return $result;
  }

  function process_row(&$row) {
    foreach ($this->objects as $item) {
      $this->objects[$item->name]->translate($row);
    }
  }

  function post_process_result($result) {
    if ($this->pager) {
      $sql = 'SELECT COUNT(*) FROM (' . $sql . ') query_summary';
      $result['total'] = db_result(db_query($sql));
    }
  }

}
