-- Equiv-HashedDistr-1

-- TPCH Query
-- with x as (select ps_suppkey from lineitem, partsupp where l_suppkey = ps_suppkey)
-- select * from x x1
-- UNION ALL 
-- select * from x x2;

-- Equiv-HashedDistr-2 
-- TPCH Query
-- with x as (select ps_suppkey from lineitem, partsupp where l_suppkey = ps_suppkey)
-- select * from x x1 inner join x2 on (ps_suppkey);

-- EquivClassesUnion

create table foo(a int, b int, c int) distributed by (a);
create table bar(x int, y int, z int) distributed by (x);

explain 
(select * from foo where a = b)
UNION
(select * from bar where y = z);

drop table foo;
drop table bar;

-- EquivClassesAndOr 

create table r(r1 int, r2 int, r3 int) distributed by (r1);
create table s(s1 int, s2 int, s3 int) distributed by (s1);
create table t(t1 int, t2 int, t3 int) distributed by (t1);

explain select * from r, t, s where (r1 = s1 AND s2 = t1 AND (s2 = r2 OR r2 = s3));

drop table r;
drop table s;
drop table t;

-- EquivClassesLimit 
create table foo(x int, y int) distributed by (x);
insert into foo select i, i%50 from generate_series(1,100) i;

explain select * from foo where ((x < 30) AND (x = y)) limit 1;

drop table foo;

-- IndexNLJoin_Cast_NoMotion;

create table R1(a varchar) WITH (appendonly=true) distributed by (a);
create table S1(b varchar) WITH (appendonly=true) distributed by (b);
create index idx1 on R1(a);
create index idx2 on S1(b);
create table R2(a varchar, primary key(a));
create table S2(b varchar, primary key(b));
insert into R1 select i from generate_series(0,10) i;
analyze R1;

explain select * from R1 inner join S1 on a = b union all select * from R2 inner join S2 on a = b;

drop table R1;
drop table R2;
drop table S1;
drop table S2;