<?php

class Link_Trellis {

  public $table_name;
  public $first_property;
  public $second_property;

  public $id_suffix = '';
  
  public function __construct($first_property, $second_property) {

    $other_table = $second_property->parent->get_plural();
    $temp = array($other_table, $first_property->parent->get_plural());
    sort($temp);
    $this->table_name = implode('_', $temp);
    $this->first_property = $first_property;
    $this->second_property = $second_property;
  }

  public function generate_join($id) {
    if (!$id)
      $id = $this->second_property->query();

    $table_name = $this->table_name;
    $first_key = $this->first_property->parent->name . $this->id_suffix;
    $second_key = $this->second_property->parent->name . $this->id_suffix;

    $sql = "JOIN $table_name ON $table_name.$first_key = " . $this->first_property->query() .
            " AND $table_name.$second_key = $id\n";
    return $sql;
  }

  public function query_rows($id) {
    if (!$id)
      $id = $this->second_property->query();

    $table_name = $this->table_name;
    $first_key = $this->first_property->parent->name . $this->id_suffix;
    $second_key = $this->second_property->parent->name . $this->id_suffix;

    $sql = "SELECT $table_name.$first_key FROM $table_name WHERE $table_name.$second_key = $id\n";

    return db_query($sql);
  }

  public function delete_row($first_id, $second_id) {
    $table_name = $this->table_name;
    $first_key = $this->first_property->parent->name . $this->id_suffix;
    $second_key = $this->second_property->parent->name . $this->id_suffix;

    $sql = "DELETE FROM $table_name WHERE $table_name.$first_key = $second_id" .
            " AND $table_name.$second_key = $first_id\n";

    return $sql;
  }

  public function generate_insert($first_id, $second_id) {
    $table_name = $this->table_name;
    $first_key = $this->first_property->parent->name . $this->id_suffix;
    $second_key = $this->second_property->parent->name . $this->id_suffix;

    $sql = "INSERT INTO $table_name ($first_key, $second_key) VALUES ($second_id, $first_id)\n;";
    return $sql;
  }

}

