-- ConstTblGetUnderSubqWithOuterRef.mdp  
CREATE TABLE foo (a int, b int);
CREATE TABLE bar (c int, d int);

SELECT * FROM foo,bar WHERE a=(SELECT c);

drop table foo;
drop table bar;
-- ConstTblGetUnderSubqWithNoOuterRef.mdp  

CREATE TABLE foo (a int, b int);
CREATE TABLE bar (c int, d int);

SELECT * FROM foo,bar WHERE a=(VALUES (1));
drop table foo;
drop table bar;

-- ConstTblGetUnderSubqUnderProjectNoOuterRef.mdp  
CREATE TABLE foo (a int, b int);

SELECT (SELECT a) FROM foo;

drop table foo;
-- ConstTblGetUnderSubqUnderProjectWithOuterRef.mdp  

EXPLAIN SELECT (VALUES (true));

-- CTG-Filter.mdp (TODO)

-- CTG-Join.mdp  

explain select * from (select 'Canada'::text as a) x, (select 'Canada'::text as b) y where a = b; 
-- Sequence-With-Universal-Outer.mdp  (TODO - UDF)
-- UseDistributionSatisfactionForUniversalInnerChild.mdp  

explain select generate_series(1,10) EXCEPT select 1;
-- Join_OuterChild_DistUniversal.mdp  

create table mrs_t1(x int);

insert into mrs_t1 select i from generate_series(1,10) i;
explain select * from (SELECT 1) x where (1 < ANY (select x from mrs_t1));

drop table mrs_t1;