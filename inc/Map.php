<?php

class Source_Field {
  public function __construct($source) {
    
    // Only copy over properties that this Source_Field already defines.
    $properties = get_object_vars($this);
    foreach ($properties as $property) {
      if (array_key_exists($property, $source)) {
        $this->{$property} = $source[$property];
      }
    }
  }
}

class Source_Table {

  public $fields = array();

  public function __construct($source) {
    foreach ($source['fields'] as $field) {
      $fields[] = new Source_Field($field);
    }
  }

}

class Map {

  public $tables = array();

  public function __construct($source) {
    $this->load_source($source);
  }

  public function load_source($source) {
    foreach ($source['tables'] as $table) {
      $this->tables[] = new Source_Table($table);
    }
  }

}

