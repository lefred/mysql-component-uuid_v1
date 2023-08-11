# mysql-component-uuid_v1

Extending MySQL using the Component Infrastructure - Adding a functions to retrieve
timestamp information related to UUID v1 (default in MySQL 8).

```
 MySQL > install component "file://component_uuid_v1";
 Query OK, 0 rows affected (0.0005 sec)
 
 MySQL > select * from performance_schema.user_defined_functions where udf_name like 'uuid%';
+------------------------+-----------------+----------+-------------+-----------------+
| UDF_NAME               | UDF_RETURN_TYPE | UDF_TYPE | UDF_LIBRARY | UDF_USAGE_COUNT |
+------------------------+-----------------+----------+-------------+-----------------+
| uuid_to_unixtime       | integer         | function | NULL        |               1 |
| uuid_to_timestamp_long | char            | function | NULL        |               1 |
| uuid_to_timestamp      | char            | function | NULL        |               1 |
+------------------------+-----------------+----------+-------------+-----------------+

 MySQL > select uuid() into @a;
 MySQL > select @a;
+--------------------------------------+
| @a                                   |
+--------------------------------------+
| cff78875-3855-11ee-bb0e-c8cb9e32df8e |
+--------------------------------------+

 MySQL > select uuid_to_unixtime(@a);
+----------------------+
| uuid_to_unixtime(@a) |
+----------------------+
|           1691765170 |
+----------------------+

 MySQL > select uuid_to_timestamp(@a);
+-------------------------+
| uuid_to_timestamp(@a)   |
+-------------------------+
| 2023-08-11 17:46:10.754 |
+-------------------------+

 MySQL > select uuid_to_timestamp_long(@a);
+------------------------------+
| uuid_to_timestamp_long(@a)   |
+------------------------------+
| Fri Aug 11 17:46:10 2023 GMT |
+------------------------------+
```

## Errors Handling

The current checks performed:

```
 MySQL > select uuid_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a");
ERROR: 3200 (HY000): uuid_to_timestamp UDF failed; This is not a UUID v1

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a1");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; Invalid UUID string length

 MySQL > select uuid_v7_to_timestamp("aaa");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; Invalid UUID string length

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a1",1);
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; this function requires only 1 parameteter

 MySQL > select uuid_v7_to_timestamp();
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; this function requires 1 parameteter
```

