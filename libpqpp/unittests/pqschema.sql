--
-- pg_dump style comment
-- Table: test; owner: comment: ;
--

/*
	 This is
	 a
	 multiline comment */

CREATE TABLE test(
		id int,
		fl numeric(5,2),
		string text,
		boolean bool,
		dt timestamp without time zone,
		ts interval);

INSERT INTO test VALUES(4, 123.45, 'some text with a ; in it and a '' too', true, '2015-04-27 23:06:03', '1 day 14:13:12');

CREATE TABLE test2(
		path text not null);

INSERT INTO test2 VALUES('$SCRIPTDIR/pqschema.sql');

CREATE FUNCTION event_tsvector() RETURNS int
LANGUAGE sql STABLE
AS $tag$
	SELECT max(id)
	FROM test
	WHERE string != 'complex '' string;';
$tag$;

CREATE TABLE bulktest(
		id int,
		string text);

