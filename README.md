# faiss-php

faiss-php is a PHP bindings for faiss.

[faiss](https://github.com/facebookresearch/faiss) A library for efficient similarity search and clustering of dense vectors.

## Requirements

libfaiss / libfaiss_c

```
$ curl -fSL "https://github.com/facebookresearch/faiss/archive/v1.4.0.tar.gz" -o "faiss-1.4.0.tgz"
$ tar xf faiss-1.4.0.tgz
$ cd faiss-1.4.0
$ ./configure --without-cuda --without-python 
$ make -j $(nproc)
$ sudo make install
$
$ cd c_api
$ make -j $(nproc)
$ sudo cp ./libfaiss_c.so /usr/local/lib/
$ sudo ldconfig
```

## Building faiss for PHP

```
$ cd faiss-php
$ phpize
$ ./configure
$ make -j $(nproc)
$ sudo make install
```

edit your php.ini and add:

```
extension=faiss.so
```

## Class synopsis

```php
Croco\faiss {
    const METRIC_INNER_PRODUCT = 0;
    const METRIC_L2            = 1;
    const FORMAT_PLAIN         = 1;
    const FORMAT_STATS         = 2;

    public __construct(int dimension[, string description, int metric])
    public boolean isTrained(voiod)
    public void add(array vectors[, int number])
    public void addWithIds(array vectors, array ids[, int number])
    public int ntotal(void)
    public array search(array query[, int k, int format, int number])
    public void reset(void)
    public void reconstruct(int key, array recons)
    public void writeIndex(string filename)
    public void readIndex(string filename)
    public void importIndex(string data)
    public string exportIndex(void)
}
```

## Table of Contents
* [Croco::faiss::__construct](#__construct)
* [Croco::faiss::isTrained](#istrained)
* [Croco::faiss::add](#add)
* [Croco::faiss::addWithIds](#addwithids)
* [Croco::faiss::ntotal](#ntotal)
* [Croco::faiss::search](#search)
* [Croco::faiss::reset](#reset)
* [Croco::faiss::reconstruct](#reconstruct)
* [Croco::faiss::writeIndex](#writeindex)
* [Croco::faiss::readIndex](#readindex)
* [Croco::faiss::importIndex](#importindex)
* [Croco::faiss::exportIndex](#exportindex)
-----

### <a name="__construct">Croco::faiss::__construct(int dimension[, string description, int metric])

Instantiates a faiss object.

```php
$index = new Croco\faiss(128, 'IDMap,Flat');

$index = new Croco\faiss(100, 'Flat', Croco::faiss::METRIC_L2);
```

-----

### <a name="istrained">void Croco::faiss::isTrained(void)

Getter for is_trained.

```php
$index = new Croco\faiss(128, 'IDMap,Flat');
$res = $index->isTrained();
var_dump($res);
```

-----

### <a name="add">void Croco::faiss::add(array data[, int number])

Add n vectors of dimension d to the index.

```php
$index = new Croco\faiss(100, 'Flat');
$vectors = [
    0.0200351,0.0941662,0.0324461,0.0755379, ......
    -0.0134401,0.00689783,0.0361747,-0.0180336, ......
    0.0744146,0.0417511,-0.0769202,0.0227152, ......
                    :
                    :
                    :
];

$object_number = 12;  // count($vectors) / dimension
$index->add($vectors, $object_number);
```

-----


### <a name="addwithids">void Croco::faiss::addWithIds(array vectors, array ids[, int number])

Same as add, but stores xids instead of sequential ids.

```php
$index = new Croco\faiss(100, 'IDMap,Flat');
$vectors = [
    [
        0.0200351,0.0941662,0.0324461,0.0755379, ......
        -0.0134401,0.00689783,0.0361747,-0.0180336, ......
        0.0744146,0.0417511,-0.0769202,0.0227152, ......
    ],
                    :
                    :
                    :
];
$ids = [
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
];

$index->addWithIds($vectors, $ids);
```

-----

### <a name="ntotal">int Croco::faiss::ntotal()

Getter for ntotal

```php
$index = new Croco\faiss(100, 'Flat');
$index->loadIndex('sample.idx');

echo $index->ntotal();
```

-----

### <a name="search">void Croco::faiss::search(array query[, int k, int format, int number])

```php
$index = new Croco\faiss(100, 'IDMap,Flat');
$vectors = [
    0.0200351,0.0941662,0.0324461,0.0755379, ......
    -0.0134401,0.00689783,0.0361747,-0.0180336, ......
    0.0744146,0.0417511,-0.0769202,0.0227152, ......
                    :
                    :
                    :
];
$ids = [
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
];

$object_number = 12;  // count($vectors) / dimension

$index->addWithIds($vectors, $ids, $object_number);


$query = [
    [
        0.0744146,0.0417511,-0.0769202,0.0227152, ......
        0.0134917,0.00398968,-0.0516475,0.0694875, ......
        -0.0531141,0.0319203,0.0229972,-0.0412282, ......
    ],
                    :
                    :
                    :
];

$res = $index->search($query, 5);
print_r($res);
```

```
[
    [0] => [
        [Rank] => 1
        [ID] => 3
        [Distance] => 0
    ],
    [1] => [
        [Rank] => 2
        [ID] => 6
        [Distance] => 0.124467253685
    ],
    [2] => [
        [Rank] => 3
        [ID] => 5
        [Distance] => 0.12547266483307
    ],
    [3] => [
        [Rank] => 4
        [ID] => 2
        [Distance] => 0.12691037356853
    ],
    [4] => [
        [Rank] => 5
        [ID] => 9
        [Distance] => 0.13562878966331
    ]
]
```


```php
$index = new Croco\faiss(100, 'IDMap,Flat');
$index->loadIndex('sample.idx');

$query = [
    [
        0.0744146,0.0417511,-0.0769202,0.0227152, ......
        0.0134917,0.00398968,-0.0516475,0.0694875, ......
        -0.0531141,0.0319203,0.0229972,-0.0412282, ......
    ],
                    :
                    :
                    :
];


$res = $index->search($query, 3, Croco\faiss\FORMAT_STATS);
print_r($res);
```

```
[
    [0] => [
        [Rank] => 1
        [ID] => 3
        [Count] => 1
        [Distance] => 0
    ],
    [1] => [
        [Rank] => 2
        [ID] => 7
        [Count] => 1
        [Distance] => 0.023740146309137
    ],
    [2] => [
        [Rank] => 3
        [ID] => 4
        [Count] => 1
        [Distance] => 0.02716763317585
    ],
    [3] => [
        [Rank] => 4
        [ID] => 6
        [Count] => 2
        [Distance] => 0.062233626842499
    ],
    [4] => [
        [Rank] => 5
        [ID] => 5
        [Count] => 1
        [Distance] => 0.12547266483307
    ]
]
```

-----

### <a name="reset">array Croco::faiss::reset()

removes all elements from the database.

```php
$index = new Croco\faiss(100, 'Flat');
$index->loadIndex('sample.idx');

$index->reset();
echo $index->ntotal();
```
-----


### <a name="reconstruct">void Croco::faiss::reconstruct(int key, array recons)

Reconstruct a stored vector (or an approximation if lossy coding).

```php
$index = new Croco\faiss(100, 'Flat');
$index->loadIndex('sample.idx');

$recons = [
    0.00097321,0.0134312,-0.0629659,0.0388441, ......
];

$index->reconstruct(3, $recons);
```

### <a name="writeindex">void Croco::faiss::writeIndex(string filename)

Write index to a file.

```php
$index = new Croco\faiss(100, 'Flat');
$vectors = [
    0.0200351,0.0941662,0.0324461,0.0755379, ......
    -0.0134401,0.00689783,0.0361747,-0.0180336, ......
    0.0744146,0.0417511,-0.0769202,0.0227152, ......
                    :
                    :
                    :
];

$object_number = 12;  // count($vectors) / dimension

$index->add($vectors, $object_number);

$index->writeIndex('index');
```

### <a name="readindex">void Croco::faiss::readIndex(string filename)

Read index from a file.

```php
$index = new Croco\faiss(100, 'Flat');
$index->readIndex('index');

$query = [
    0.0744146,0.0417511,-0.0769202,0.0227152, ......
    0.0134917,0.00398968,-0.0516475,0.0694875, ......
    -0.0531141,0.0319203,0.0229972,-0.0412282, ......
                    :
                    :
                    :
];

$res = $index->search($query, 5, Croco\faiss\FORMAT_PLAIN, 1);
```


### <a name="importindex">void Croco::faiss::importIndex(string indexdata)

Import index.


```php
$db = new \PDO(......);
$stmt = $db->prepare('SELECT `index` FROM `faiss` WHERE `id` = :id');
$stmt->bindValue(':id', 5, \PDO::PARAM_INT);
$stmt->execute();

$data = $stmt->fetchColumn();

$index = new Croco\faiss(100, 'IDMap');
$index->importIndex($data);

$query = [
    [
        0.0744146,0.0417511,-0.0769202,0.0227152, ......
        0.0134917,0.00398968,-0.0516475,0.0694875, ......
        -0.0531141,0.0319203,0.0229972,-0.0412282, ......
    ],
                    :
                    :
                    :
];

$res = $index->search($query, 5);

```

-----


### <a name="exportindex">void Croco::faiss::exportIndex()

Export index.


```php
$db = new \PDO(......);

$index = new Croco\faiss(100, 'Flat');
$vectors = [
    [
        0.0200351,0.0941662,0.0324461,0.0755379, ......
        -0.0134401,0.00689783,0.0361747,-0.0180336, ......
        0.0744146,0.0417511,-0.0769202,0.0227152, ......
    ],
                    :
                    :
                    :
];
$index->add($vectors);


$stmt = $db->prepare('INSERT INTO `faiss` (`index`)VALUES(:index)');
$stmt->bindValue(':index', $index->exportIndex(), \PDO::PARAM_LOB);
$stmt->execute();
```

-----

#### Create Table
```sql
CREATE TABLE `faiss` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `index` mediumblob NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=ascii COLLATE=ascii_bin
```
