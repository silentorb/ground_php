#Ground PHP

Copyright 2013 Silent Orb

Ground is a server-side, schema-based framework for data modeling.  It is sort of an ORM, though it avoids certain features ORMs nromally contain, and in return contains certain features ORMs normally don't have.  Ground is designed to be used in the larger context of the Vineyard framework, though much of Ground can be utilized without Vineyard.

Since Ground makes the most sense within the context of Vineyard, and since Vineyard is really the joining of projects without a project of its own, I will explain Vineyard here.

Vineyard is a framework for auto generating web services and user interfaces from data schemas.  The problem of bridging the gap between server and client is not a new one, and many frameworks have been created to solve it, but Vineyard provides a unique combination of features that gives it an edge.

The Schema Layers
-----------------

One of Vineyard's biggest strengths is that instead of having a single schema for defining its data, it divides its schema into three layers.  The middle layer is the initial and primary layer that defines all of the objects and their properties.  It is known as the Vineyard Layer.  The bottom layer defines database specific attributes for how the objects and properties map to tables and fields.   It is known as the Ground Layer.  The top layer defines how those objects and properties appear to the user and can be interacted with.  That is known as the Bloom Layer.

These schemas are stored in JSON files.  Ground can load any number of schema files and use them together.  Each schema file can contain any combination of three different lists of objects:

1. Tables (for the Ground layer)
2. Trellises (for the Vineyard layer)
3. Arbors (for the Bloom layer)

In some cases all that is needed is the Vineyard layer.  Both Ground and Bloom have default conventions for how to interpret the Vineyard schema, and their particular schema layers are only needed to override the default behavior.  For example, if the Vineyard Schema defines an object type called "book", Ground assumes that there must be a table in the database named "books".  If the corresponding table is actually named "book_list", a Ground schema will be needed to override the table name for "books".

API Reference
-------------

# class Ground extends Meta_Object 

static function load_json_from_file($filename)
----------------------------------------------
### Arguments

*$filename*: String path to a JSON file.

### Returns

Returns a JSON-decoded Object.

### Description

Loads data from a JSON file.


function convert_value($value, $type)
----------------------------------------------

### Arguments

*$value*: Value to be converted.
*$type*: Vineyard type that $value should be converted to.

### Returns

Returns the converted value.

### Description

Converts a variable to a specific Vineyard type.


function insert_object($trellis, $data = array())
----------------------------------------------

### Arguments

*$trellis*: The Trellis that defines the object.  Can be either the name of a Trellis or Trellis object.
*$data = array()*: A list of property values to initialize the object with.

### Returns

Returns a new object of the specified Trellis type.

### Description

Inserts object records into the database and returns the inserted object.  This is a wrapper around the Update class.  This function can be used to create new records or update existing ones.

### Example

```php

$object = $ground->insert_object('tree', array(
  'name' => 'pine',
  'status' => 'burning',
));

``` 

## class Trellis 

function is_a($name)
--------------------

### Arguments

*$name*: String name of a trellis.

### Returns

Returns true or false.

### Description

Determines whether the calling Trellis is the named Trellis or a descendant of the named Trellis.

### Example

```php
if ($trellis->is_a('frog');
  return 'kiss me';
``` 
