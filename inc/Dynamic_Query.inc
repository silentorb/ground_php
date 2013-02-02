<?php

class Dynamic_Query {

  protected $objects = array();
  public $main_table = 'node';
  public $main_table_alias = null;
  public $field_sql;
  public $main_sql;
  protected $pre_clauses = array();
  protected $post_clauses = array();
  protected $order = array();
  protected $filters = array();
  protected $fields = array();
  protected $filter_fields = array();
  protected $joins = array();

  public function __construct($main_table) {
    $this->main_table = $main_table;
  }

  function add_filter($clause) {
    $this->filters[] = $clause;
  }

  function add_field($clause) {
    $this->fields[] = $clause;
  }

  function add_filter_field($clause) {
    $this->filter_fields[] = $clause;
  }

  function add_join($clause) {
    $this->joins[] = $clause;
  }

  function add_pre($clause) {
    $this->pre_clauses[] = $clause;
  }
  
  function add_post($clause) {
    $this->post_clauses[] = $clause;
  }

  function add_order($clause) {
    $this->order[] = $clause;
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

    $fields = array_merge($this->fields, $this->filter_fields);
    foreach ($this->objects as $object) {
      $field = $object->render_fields();
      if ($field)
        $fields[] = $field . "\n";
    }
    $this->field_sql = implode(', ', $fields);

    $main_sql = ' FROM ' . $this->main_table . ' ';
    if ($this->main_table_alias) {
      $main_sql .= 'AS ' . $this->main_table_alias . ' ';
    }
    $main_sql .= ' ' . implode(' ', $this->joins);

    if (count($this->filters) > 0) {
      $main_sql .= ' WHERE ' . implode(' AND ', $this->filters);
    }
    $main_sql .= ' ' . implode(' ', $this->post_clauses);

    $this->main_sql = $main_sql;
    
    $pre_sql = '';
    if (count($this->pre_clauses))
      $pre_sql = implode(' ', $this->pre_clauses) . ' ';

    $sql = $pre_sql . 'SELECT ' . $this->field_sql . $main_sql;

    if (count($this->order))
      $sql .= ' ORDER BY ' . implode(' ', $this->order);

    return $sql;
  }

  function run($return_sql = false) {
    $result = new stdClass();
    $result->nodes = array();
    $sql = $this->generate_sql();
    $sql = str_replace("\r", "\n", $sql);
    $result->timer = array();
    $paged_sql = $this->paged_sql($sql);
    $start_time = microtime(true);

//    $result->sql = $paged_sql;
//    return $result;
    $rows = db_query($paged_sql);
    $result->timer['main'] = microtime(true) - $start_time;
    $result->timer['total'] = $result->timer['main'];
    while ($row = db_fetch_object($rows)) {
      $this->process_row($row);
      $result->nodes[] = $row;
    }

    $this->post_process_result($result, $sql);

    if ($return_sql)
      $result->sql = $paged_sql;

    return $result;
  }

  function run_simple() {
    $sql = $this->generate_sql();
    return db_query($sql);
  }

  function process_row(&$row) {
    foreach ($this->objects as $item) {
      $this->objects[$item->name]->translate($row);
    }
  }

  function post_process_result($result, $sql) {
    if ($this->pager) {
      $fields = array_merge(array('node.nid'), $this->filter_fields);
      $field_string = implode(', ', $fields);
      $sql = "SELECT COUNT(*) FROM (SELECT $field_string" . $this->main_sql . ')';
//      $result->count_sql = $sql;
      $start_time = microtime(true);
      $result->total = db_result(db_query($sql));
      $result->timer['count'] = microtime(true) - $start_time;
      $result->timer['total'] += $result->timer['count'];
    }
  }
}
