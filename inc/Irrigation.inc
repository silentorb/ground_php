<?php

class Channel {

  public $path;
  public $target;
  public $method;

  public function __construct($path, $target, $method) {
    if (is_string($path))
      $this->path = Irrigation::convert_path_to_array($path);
    else {
      $this->path = $path;
    }

    $this->target = $target;
    $this->method = $method;
  }

  public function run($bag) {
    return $this->target->{$this->method}($bag);
  }

}

class Bag {

  public $path;
  public $arguments;
  public $objects;

  public function __construct($path = null) {
    if (is_string($path))
      $this->path = Irrigation::convert_path_to_array($path);
    else if (is_array($path))
      $this->path = $path;
    else
      $this->path = array();
  }
}

class Irrigation {

  public static $base_path = 'vineyard';
  public $channels = array();
  // Not used yet.  Will be used for optimized routing.
  public $tree = array();
  // Ground is optional.
  public $ground;

  public function __construct($ground = null) {
    $this->ground = $ground;
  }

  public function add_channel($path, $target, $method) {
    $channel = new Channel($path, $target, $method);
    $this->channels[] = $channel;
  }

  public function generate_tree() {
    // Later
  }

  public function compare($pattern, $text) {
    if (!$pattern || !$text)
      return false;

    if ($pattern == '%')
      return true;

    if ($pattern == '@trellis') {
      if ($this->ground)
        return array_key_exists($text, $this->ground->trellises);
      else
        return false; // Should an exception be thrown here?
    }

    return $pattern == $text;
  }

  public function process($path, $bag) {
    $channel = $this->find_channel($path);

    if (!$channel) {
      throw new Exception('Path not found', 404);
    }

    $channel->run($bag);
  }

  public function find_channel($path) {
    if (is_string($path))
      $path = Irrigation::get_path_array($path, Irrigation::$base_path);

    // We're going to utilize PHP's copying arrays by value.
    $channels = array();
    foreach ($this->channels as $channel) {
      if (count($channel->path) == count($path))
        $channels[] = $channel;
    }

    $x = 0;
    foreach ($path as $step) {
      $next_channels = array();
      foreach ($channels as $channel) {
        if ($this->compare($channel->path[$x], $step))
          $next_channels[] = $channel;
      }
      $channels = $next_channels;
      ++$x;
    }

    return $channels[0];
  }

  static function create_bag_from_globals() {
    $bag = new Bag();
    $bag->path = Irrigation::get_path_array($_SERVER["REQUEST_URI"], Irrigation::$base_path);
    $bag->arguments = $_REQUEST;
    $bag->objects = Irrigation::get_post_objects();

    return $bag;
  }

  static function get_post_objects() {
    $objects = array();
    if (isset($_POST['objects'])) {
      $objects = $_POST['objects'];
    }
    else {
      $json = file_get_contents("php://input");
      $data = json_decode(utf8_encode($json));
      if ($data)
        $objects = $data->objects;
    }

    return $objects;
  }

  static function convert_path_to_array($path) {
    if (!$path || strlen($path) == 0)
      return array();

    if ($path[0] == '/')
      $path = substr($path, 1);

    if (substr($path, strlen($path) - 1) == '/')
      $path = substr($path, 0, strlen($path) - 1);

    return explode('/', $path);
  }

  static function get_path_array($path, $base) {
    $path = Irrigation::convert_path_to_array($path);
    $base = Irrigation::convert_path_to_array($base);

    $amount = 0;
    foreach ($base as $i => $item) {
      if ($item == $path[$i]) {
        $amount++;
      }
      else {
        break;
      }
    }

    if ($amount)
      array_splice($path, 0, $amount);

    return $path;
  }

}
