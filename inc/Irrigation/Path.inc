<?php

namespace ground\irrigation;

class Path {

  public $text;
  public $tokens = array();

  public function __construct($input) {
    if (is_string($input)) {
      $this->text = $input;
      $this->tokens = \Irrigation::get_path_array($input);
    }
    else if (is_array($input)) {
      $this->tokens = $input;
      $this->text = implode('/', $input);
    }
    else {
      throw new \Exception('Invalid argument in path comparison.');
    }
  }

  public function match($input) {
    if (is_string($input)) {
      return $this->text == $input;
    }
    else if (is_array($input)) {
      return \Irrigation::path_arrays_match($this->tokens, $input);
    }
    else if (get_class($input) == 'Path') {
      return \Irrigation::path_arrays_match($this->tokens, $input->tokens);
    }
    else {
      throw new \Exception('Invalid path argument.');
    }
  }

}