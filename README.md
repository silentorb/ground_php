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

Background
----------

In explaining Ground, it is useful to relate some of my background.  My roots are in straight client-side programming.  All of a program's data was stored in either binary or text files distributed with the program.  When I became involved in web programming, and in doing so was introduced to databases, I was horrified.  SQL seemed clunky and tedious compared to the object oriented, XML heavy world I had come from.  So I quickly gravitated to ORMs and tried to hide the database as much as possible behind abstractions.  I kept trying different ORM frameworks, but none of them seemed to adequately hide the databases, and they generally left me wanting more.

Then I got much deeper into web development and found myself having to deal with SQL on a daily basis.  After months of this I began to appreciate SQL more.  Eventually I grew to love SQL and realized the main problem I had run into with ORMs was that I needed tools that focused on streamlining development for the database instead of hiding it.  Most major ORMs contain features and design decisions that are very heavy and come with a high price tag such as:

* Change tracking
* Reference tracking
* Lazy loading
* Auto generated code
* Monolithic schemas
