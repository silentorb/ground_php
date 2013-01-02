<?php

class Query {

  protected $trellises = array();
  public $main_table = 'node';
  public $joins = array();
  public $filters = array();
  public $post_clauses = array();
  public $limit = '';
  public $sources = array();
  public $links = array();
  public $trellis;
  public $db;

  public function __construct($trellis, $include_links = true) {
    if (get_class($trellis) != 'Trellis' && !is_subclass_of($trellis, 'Trellis')) {
      throw new Exception('An invalid trellis was passed to the Query constructor.');
    }
    $this->trellis = $trellis;
    $this->ground = $this->trellis->ground;
    $this->db = $this->ground->db;
    $this->main_table = $trellis->get_plural();
    $this->add_source($trellis);
    if ($include_links) {
      $current_trellis = $trellis;
      do {
        foreach ($current_trellis->links as $link) {
          $this->add_link($link);
        }
      }
      while ($current_trellis = $current_trellis->parent);
    }
  }

  function add_source($source, $include_primary_key = true) {
    $this->sources[] = $source;
    $table_name = $source->get_plural();
    foreach ($source->core_properties as $property) {
      if ($property->name != $source->primary_key || $include_primary_key)
        $this->add_field($table_name . '.' . $property->field_name);
    }

    $source->parent_query($this);
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

  function add_link($property) {
    if (is_string($property)) {
      $name = $property;
      $property = $this->trellis->properties[$name];
      if (!$property)
        throw new Exception($this->trellis->name . ' does not have a property named ' . $name . '.');
    }

    $other = $this->ground->trellises[$property->trellis];
    if (!$other)
      throw new Exception('Could not find reference to property ' . $property->name . ' for ' . $property->trellis . '.');

    $link = new stdClass();
    $link->other = $other;
    $link->property = $property;
    $this->links[$property->name] = $link;

    if ($property->type == 'reference') {
      $this->add_field("$property->field_name AS `$property->name`");
    }
  }

  function add_links($paths) {
    foreach ($paths as $path) {
      $this->add_link($path);
//      $levels = explode('/', $path);
//      $last_link = null;
//      foreach($levels as $level) {
//        $link = array();
////        if ($last_link)
////        $link['child']
//      }
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
    if ($this->trellises[$table])
      unset($this->trellises[$table]->fields[$field_name]);
  }

  function generate_sql() {
    global $user;

    $sql = 'SELECT ';
    $sql .= implode(', ', $this->fields);

    $sql .= ' FROM ' . $this->main_table;

    if (count($this->joins) > 0) {
      $sql .= ' ' . implode(' ', $this->joins);
    }

    if (count($this->filters) > 0) {
      $sql .= ' WHERE ' . implode(' AND ', $this->filters);
    }

    if (count($this->post_clauses) > 0) {
      $sql .= ' ' . implode(' ', $this->post_clauses);
    }

    return $sql;
  }

  function run($return_sql = false) {
    $result = new stdClass();
    $result->objects = array();
    $sql = $this->generate_sql();
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);
    file_put_contents('debug.sql', $paged_sql);

    $this->sql = $paged_sql;
    $rows = $this->db->query_objects($paged_sql);
    foreach ($rows as $row) {
      $this->process_row($row);
      $result->objects[] = $row;
    }

    $this->post_process_result($result);

    if ($return_sql)
      $result->sql = $this->sql;

    return $result;
  }

  function process_row(&$row) {
    // Map field names to bloom property names.
    foreach ($this->trellis->properties as $property) {
      if ($property->name != $property->field_name) {
        if (isset($row->{$property->field_name})) {
          $row->{$property->name} = $row->{$property->field_name};
          unset($row->{$property->field_name});
        }
      }
    }

    foreach ($this->trellises as $item) {
      $this->trellises[$item->name]->translate($row);
    }

    foreach ($this->sources as $source) {
      foreach ($source->properties as $property) {
        $full_name = $property->name;

        if (property_exists($row, $full_name)) {
          $row->$full_name = Ground::convert_value($row->$full_name, $property->type);
        }
      }
    }

    foreach ($this->links as $name => $link) {
      $property = $link->property;
      $id = $row->{$property->parent->primary_key};
      $other_property = $link->other->get_link_property($property->parent);
      if ($other_property === null)
        throw new Exception($property->parent->name . '->' . $property->name . ' does not have a reciprocal reference on ' . $link->other . '.');

      // First check for Many to Many
      if ($property->type == 'list' && $other_property->type == 'list') {
        $query = $this->ground->create_query($link->other, false);
        $join = new Link_Trellis($link->other->get_primary_property(), $property->parent->get_primary_property());
        $join_sql = $join->generate_join($id);
        $query->add_join($join_sql);

        $result = $query->run(true);
        $this->sql .= "\n" . $result->sql;

//        if ($link->property->type == 'reference')
//          $row->{$name} = $result->objects[0];
//        else
        $row->{$name} = $result->objects;
      }
      else if ($property->type == 'list') {
        // One to Many
        $query = $this->ground->create_query($link->other, false);
        $query->add_filter($other_property->query() . ' = ' . $id);
        $result = $query->run(true);
        $this->sql .= "\n" . $result->sql;
        $row->{$name} = $result->objects;

//        if ($other_property->type == 'list') {
        $row->{$name} = $result->objects;
//        }
//        else { // One to One
//          $row->{$name} = $result->objects[0];
//        }
      }
      else {
        $query = $this->ground->create_query($link->other, false);
        $query->add_filter($link->other->query_primary_key() . ' = ' . $row->{$property->name});
        $result = $query->run(true);
        $this->sql .= "\n" . $result->sql . "($property->name)";

        $row->{$name} = $result->objects;
      }
    }
  }

  function post_process_result($result) {
  }

}
