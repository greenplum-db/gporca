-- InferPredicatesForLimit.mdp
create table foo (a int, b int) distributed by (a);
explain select 1 from (select * from foo where a = 1 and b = a limit 1) x, foo where x.a = foo.b and x.a < 2;

drop table foo;
-- ArrayCoerceExpr.mdp

create table foo (c int, d character varying(10));
explain select * from foo where d in ('a', 'b');

drop table foo;
-- SelectOnCastedCol.mdp

-- TPCDS
-- select * from customer_address where ca_country = "USA";

-- JOIN-Pred-Cast-Int4.mdp
create table aint (i int);
create table bbigint (i bigint);
insert into aint select i from generate_series(1,100) i;
insert into bbigint select i*10000000 from generate_series(1,100) i;
explain select * from aint, bbigint where aint.i = bbigint.i;

drop table aint;
drop table bbigint;
-- JOIN-Pred-Cast-Varchar.mdp 
create table a (i int, j varchar(10));
create table b (i int, j varchar(10));

insert into a select i, 'x' from generate_series(1,100) i;
insert into b select i, 'x' from generate_series(1,100) i;

explain select * from a, b where a.j::text = b.j::text;
drop table b;
drop table a;
-- JOIN-int4-Eq-double.mdp
create table aint (i int);
create table cdouble (i numeric(10,10));
insert into aint select i from generate_series(1,100) i;
insert into cdouble select i from generate_series(1,100) i;

explain select * from aint, cdouble where aint.i = cdouble.i;

drop table aint;
drop table cdouble;

-- JOIN-cast2text-int4-Eq-cast2text-double.mdp

create table aint (i int);
create table cdouble (i numeric(10,10));
insert into aint select i from generate_series(1,100) i;
insert into cdouble select i from generate_series(1,100) i;

explain select * from aint, cdouble where aint.i::text = cdouble.i::text;

drop table aint;
drop table cdouble;

-- JOIN-int4-Eq-int2.mdp

create table aint (i int);
create table dint2 (i smallint);
insert into aint select i from generate_series(1,100) i;
insert into cdouble select i from generate_series(1,100) i;

explain select * from aint, cdouble where aint.i=dint2.i;

drop table aint;
drop table cdouble;

-- CastOnSubquery.mdp 
create table foo (a int, b int);
create table bar (c int, d int);

insert into foo select i, i from generate_series(1,10) i;
insert into bar select i, i from generate_series(1,10) i;
explain select * from foo where foo.a::text ~~ (select bar.c::TEXT from bar);

drop table foo;
drop table bar;
-- CoerceToDomain.mdp 

explain SELECT * FROM information_schema.tables;

-- CoerceViaIO.mdp 

CREATE TABLE foo (id int4);
explain SELECT * FROM foo WHERE id::text = 'foo';
drop table foo;

-- ArrayCoerceCast.mdp

CREATE TABLE FOO(a integer NOT NULL, b double precision[]);

SELECT b FROM foo UNION ALL SELECT ARRAY[90, 90] as Cont_features;

drop table foo;

-- SimpleArrayCoerceCast.mdp

explain SELECT ARRAY[1.11, 2.11] UNION ALL SELECT ARRAY[1, 2];

-- EstimateJoinRowsForCastPredicates.mdp 

create table A(i int , j varchar(10)) distributed by (i);
create table B(i int , j varchar(10)) distributed by (i);
insert into A select i, 'a0' from  generate_series(1,100) i;
insert into B select i, 'a0' from  generate_series(1,100) i;
insert into B select i, 'a1' from  generate_series(1,100) i;

explain select * from A,B where A.j=B.j;

drop table A;
drop table B;

-- HashJoinOnRelabeledColumns.mdp

CREATE TABLE foo (a varchar(16));
CREATE TABLE bar (b varchar(16));

explain select * from foo, bar where a::text = b::text;

drop table foo;
drop table bar;
-- Correlation-With-Casting-1.mdp

create table q68t792_temp(u_vtgnr varchar(6), u_zj varchar(2), u_folio varchar(2)) distributed by (u_vtgnr, u_zj, u_folio);
insert into q68t792_temp select x ||'1' , x || '', x || '' from generate_series(30,99) x;

SELECT  u_vtgnr, u_zj, u_folio                                                                                                                                 
FROM  q68t792_temp a  
WHERE 
u_zj = 
(SELECT substr(max(case when cast(u_zj as integer) < 50 then '20' || u_zj else '19' || u_zj end),3,2) 
 FROM q68t792_temp b WHERE a.u_vtgnr = b.u_vtgnr) 
AND u_folio = (SELECT max(u_folio)  FROM q68t792_temp c WHERE a.u_vtgnr = c.u_vtgnr  and a.u_zj = c.u_zj);

drop table q68t792_temp;
-- Correlation-With-Casting-2.mdp

create table q68t792_temp(u_vtgnr varchar(6), u_zj varchar(2), u_folio varchar(2)) distributed by (u_vtgnr, u_zj, u_folio);
insert into q68t792_temp select x ||'1' , x || '', x || '' from generate_series(30,99) x;

SELECT  u_vtgnr, u_zj, u_folio                                                                                                                                 
FROM  q68t792_temp a  
WHERE 
u_zj = 
(SELECT substr(max(case when cast(u_zj as integer) < 50 then '20' || u_zj else '19' || u_zj end),3,2) 
 FROM q68t792_temp b WHERE a.u_vtgnr <> b.u_vtgnr) 
AND u_folio = (SELECT max(u_folio)  FROM q68t792_temp c WHERE a.u_vtgnr <> c.u_vtgnr  and a.u_zj <> c.u_zj);

drop table q68t792_temp;
-- Date-TimeStamp-HashJoin.mdp 

create table d1 (i int, d date);
create table d2 (i int, d timestamp);

explain select * from d1, d2 where d1.d = d2.d;

drop table d1;
drop table d2;

-- TimeStamp-Date-HashJoin.mdp 

create table d1 (i int, d date);
create table d2 (i int, d timestamp);

explain select * from d1, d2 where d2.d = d1.d;

drop table d1;
drop table d2;

