-- DQA-KeepOuterReference.mdp 
CREATE TABLE tint (c int);
select (select distinct tint.c from tint as tt)  x from tint;
drop table tint;

-- ScalarSubqueryCountStarInJoin.mdp 
CREATE TABLE foo(a,b) AS VALUES (1,1);
CREATE TABLE bar(c int, d int);
CREATE TABLE jazz(e int, f int);

SELECT (SELECT jazz.count FROM (SELECT count(*) FROM bar GROUP BY c LIMIT 1) AS bar, (SELECT count(*) FROM jazz WHERE e = a) AS jazz) FROM foo;

drop table foo;
drop table bar;
drop table jazz;
-- ScalarCorrelatedSubqueryCountStar.mdp 

CREATE TABLE foo(a, b) AS VALUES (1, 1);
CREATE TABLE bar(c int, d int);
EXPLAIN SELECT (SELECT count(*) FROM bar where c = a) FROM foo;

drop table foo;
drop table bar;
-- ScalarSubqueryCountStar.mdp 

CREATE TABLE foo (a, b) AS SELECT i,i FROM generate_series(1,1) AS t(i) DISTRIBUTED BY(a);
CREATE TEMP TABLE bar (c int, d int);
EXPLAIN SELECT (SELECT COUNT(*) FROM bar GROUP BY c LIMIT 1) FROM foo;

drop table foo;
drop table bar;
-- DQA-SplitScalarWithAggAndGuc.mdp 
create table foo (a int, b int) distributed by (a);
insert into foo select i, i%2 from generate_series(1, 100000) i;
set optimizer_force_three_stage_scalar_dqa = on;

explain select count(distinct b ), sum(b) from foo;
drop table if exists foo;

-- DQA-SplitScalarWithGuc.mdp
create table foo (a int, b int) distributed by (a);
insert into foo select i, i%2 from generate_series(1, 100000) i;

set optimizer=on;
set optimizer_prefer_scalar_dqa_multistage_agg=on;
explain select count(distinct b ) from foo;
-- DQA-SplitScalar.mdp 
drop table if exists foo;
create table foo (a int, b int) distributed by (a);

insert into foo select i, i%2 from generate_series(1, 100000) i;

set optimizer=on;
set optimizer_segments=128;

explain select count(distinct b ) from foo;

drop table if exists foo;
-- Agg-NonSplittable.mdp (TODO --- too complex)
-- SortOverStreamAgg.mdp (TODO)
-- NoHashAggWithoutPrelimFunc.mdp 

CREATE TABLE attribute_table (product_id integer, attribute_id integer,attribute text, attribute2 text,attribute_ref_lists text,short_name text,attribute6 text,attribute5 text,measure double precision,unit character varying(60)) DISTRIBUTED BY (product_id ,attribute_id);
CREATE OR REPLACE FUNCTION do_concat(text,text) 
RETURNS text 
AS 'SELECT CASE WHEN $1 IS NULL THEN $2
WHEN $2 IS NULL THEN $1
ELSE $1 || $2 END;'
     LANGUAGE SQL 
     IMMUTABLE 
     RETURNS NULL ON NULL INPUT; 

CREATE AGGREGATE concat(text) ( 
    SFUNC = do_concat, 
    STYPE = text,
    INITCOND = '' 
);

select product_id,concat('#attribute_'||attribute_id::varchar||':'||attribute) as attr FROM attribute_table GROUP BY product_id;

-- AggWithSubqArgs.mdp (TODO)
create table a (a1 int, a2 int) distributed by (a1);
create table b (b1 int, b2 int) distributed by (b1);

-- explain select sum(select a1 from a where a2 = b2) from b;

drop table a;
drop table b;
-- Agg-Limit.mdp 
create table x (i int, j int) distributed by (i);
insert into foo select i, i%2 from generate_series(1,10) i;
explain select sum(i) from (select * from x limit 10) x(i,j);

drop table x;
-- GroupByEmptySetNoAgg.mdp 
create table gpd (a int, b int) distributed by (a);
insert into gpd select i, i%10 from generate_series(1,2) i;
explain select 1 from gpd;

drop table gpd;
-- CollapseGb-With-Agg-Funcs.mdp 
create table x (i int, j int) distributed by (i);

explain select i from (select i, j, sum(i) as s from x group by i,j) x(i, j, s) group by i, s order by i, s;

drop table x;
-- CollapseGb-Without-Agg-Funcs.mdp 
create table x (i int, j int) distributed by (i);

explain select i from (select i,j from x group by i,j) f(i) group by i order by i;

drop table x;
-- CollapseGb-SingleColumn.mdp 
create table foo (i int, j int) distributed by (i);

insert into foo select i, i%10 from generate_series(1,10000) i;
explain select i from (select i from foo group by i) x(i) group by i,j order by i;

drop table foo;
-- CollapseGb-MultipleColumn.mdp 
create table foo (i int, j int) distributed by (i);

insert into foo select i, i%10 from generate_series(1,10000) i;
explain select i,j from (select i,j from foo group by i,j order by i,j) x(i,j) group by i,j order by i,j;

drop table foo;
-- CollapseGb-Nested.mdp 
create table foo (i int, j int) distributed by (i);

insert into foo select i, i%10 from generate_series(1,10000) i;
explain select i from (select i from foo group by i order by 1) x(i) group by i order by i;

drop table foo;
-- ThreeStageAgg.mdp 

create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(c) from r group by b;

drop table r;
-- ThreeStageAgg-GbandDistinctOnDistrCol.mdp 

create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct a) from r group by a;

drop table r;
-- ThreeStageAgg-GbMultipleCol-DistinctOnDistrCol.mdp 

create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct a) from r group by b, c;

drop table r;
-- ThreeStageAgg-DistinctOnSameNonDistrCol.mdp 

create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct b) from r group by b;

drop table r;
-- ThreeStageAgg-DistinctOnDistrCol.mdp 
create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct a) from r group by b;

drop table r;
-- ThreeStageAgg-ScalarAgg-DistinctDistrCol.mdp 
create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct a) from r;

drop table r;
-- ThreeStageAgg-ScalarAgg-DistinctNonDistrCol.mdp 

create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct b) from r;

drop table r;
-- ThreeStageAgg-ScalarAgg-DistinctComputedCol.mdp 

create table r (a int, b int, c int) distributed by (a);

insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;
explain select sum(distinct (c + a)) from r;

drop table r;

-- CannotPullGrpColAboveAgg.mdp 
-- DQA-1-RegularAgg.mdp 

create table r (a int, b int, c int) distributed by (a);
insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;

explain select count(distinct a) from r group by b;

drop table r;
-- DQA-2-RegularAgg.mdp 

create table r (a int, b int, c int) distributed by (a);
insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;

explain select count(distinct a), sum(a), avg(c) from r group by b;

drop table r;
-- MDQA-SameDQAColumn.mdp 
create table r (a int, b int, c int) distributed by (a);
insert into r select i%2, i%5, i%5 from generate_series(1,1001232) i;

explain select count(distinct (a + b)), sum( distinct (a+b)), avg(a+b) from r group by c;

drop table r;
-- MDQAs1.mdp 

create table t1(a int, b int) Distributed by (a);
explain select sum(distinct a), avg(distinct b) from t1;

drop table t1;
-- MDQAs-Grouping.mdp 

create table t1(a int, b int) Distributed by (a);
explain select a, count(distinct a), count(distinct b) from t1 group by a;

drop table t1;
-- MDQAs-Grouping-OrderBy.mdp 

create table t1(a int, b int) Distributed by (a);
select a, count(distinct a), count(distinct b) from t1 group by a order by a asc limit 10;

drop table t1;
-- MDQAs-Union.mdp 

create table t1(a int, b int) Distributed by (a);

select a,count(distinct a), count(distinct b) from t1 group by a
union
select a,count(distinct a), count(distinct b) from t1 group by a;

drop table t1;
-- DistinctAgg-NonSplittable.mdp 

create table onek(
unique1 int,
unique2 int,
two int,
four int,
ten int,
twenty int,
hundred int,
thousand int,
twothousand int,
fivethous int,
tenthous int,
odd int,
even int,
stringu1 text,
stringu2 text,
string4 text);

-- NEEDS MORE THOUGHT, SINCE THE AVERAGE IS A USER DEFINED NON SPITTABLE AGG.
drop table onek;

-- RollupNoAgg.mdp (TODO)
-- Rollup.mdp  (TODO)
-- GroupingSets.mdp  (TODO)
-- CapGbCardToSelectCard.mdp 

-- TPCDS SQL:
-- select item.i_item_id, item.i_item_desc, item.i_current_price
-- from item, store_sales
-- where ss_item_sk=i_item_sk and  i_current_price &gt= 62::numeric AND i_current_price &lt= 92::numeric AND (i_manufact_id = ANY (ARRAY[129, 270, 821, 423]))
-- group  by item.i_item_id, item.i_item_desc, item.i_current_price;

-- GroupingOnSameTblCol-1.mdp 

-- TPCDS SQL:
-- select item.i_item_id, item.i_item_desc, item.i_current_price
-- from item, store_sales
-- where ss_item_sk=i_item_sk and  i_current_price &gt;= 62::numeric AND i_current_price &lt;= 92::numeric AND (i_manufact_id = ANY (ARRAY[129, 270, 821, 423]))
-- group  by item.i_item_id, item.i_item_desc, item.i_current_price;

-- GroupingOnSameTblCol-2.mdp 
-- NEEDS MORE THOUGHT

-- PushGbBelowJoin-NegativeCase.mdp 

create table t39(c499 int, c500 int, c501 int, c502 int, c503 int, c504 int) distributed by (c499);

explain select * from (select avg(c504) as a, sum(c504) as s, max(c504) as m from t39 group by c504)x, (select 1) y(c) where a = c;

drop table t39;
-- Gb-on-keys.mdp 

create table keys (i int, j int primary key, k int);

explain select i, j from keys group by i, j;

drop table keys;
-- ComputedGroupByCol.mdp 

create table r (a int, b int, c int, d int);
insert into r select i%10, i%15, i, i from generate_series(1,1000000) i;
explain select count(*) from r group by (a+b);

drop table r;
-- GroupByOuterRef.mdp 

create table t (a int, b int);
create table s (i int, j int);

explain select a, b from t where t.a in (select count(s.i) from s group by t.b);

drop table t;
drop table s;
-- DuplicateGrpCol.mdp 

create table foo (a int, b int);
insert into foo select i, i%20 from generate_series(1,1000000) i;
explain select count(*) from foo group by b, (b + 1);

drop table foo;
-- CountAny.mdp 

create table x(i int, j int);
create table y(i int, j int);
insert into x select i, i from generate_series(1,10000) i;
insert into y select i%10, i%2 from generate_series(1,10) i;

explain select 1 + (select count(i) from x) from y;

drop table x;
drop table y;
-- CountStar.mdp 

create table x(i int, j int);
create table y(i int, j int);
insert into x select i, i from generate_series(1,10000) i;
insert into y select i%10, i%2 from generate_series(1,10) i;

explain select (select count(*) from x) from y;

drop table x;
drop table y;
-- ProjectCountStar.mdp 

create table x(i int, j int);
create table y(i int, j int);
insert into x select i, i from generate_series(1,10000) i;
insert into y select i%10, i%2 from generate_series(1,10) i;

explain SELECT (SELECT 1 + count(*) FROM x) FROM y;

drop table x;
drop table y;
-- ProjectOutsideCountStar.mdp 

create table x(i int, j int);
create table y(i int, j int);
insert into x select i, i from generate_series(1,10000) i;
insert into y select i%10, i%2 from generate_series(1,10) i;

explain select 1 + (select count(*) from x) from y;

drop table x;
drop table y;
-- NestedProjectCountStarWithOuterRefs.mdp

create table x (i int, j int) distributed by (i);
create table y (i int, j int) distributed by (i);

insert into x select i, i from generate_series(1,10000) i;
insert into y select i, i%2 from generate_series(1,10) i;

explain select (select c+1 from (select count(*) +1 from x where x.j = y.j) z(c)) from y;

drop table x;
drop table y;