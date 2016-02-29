
CREATE TABLE outer_table (
    flex_value_id numeric(15,0) ,
    language character varying(4) ,
    last_update_date date ,
    last_updated_by numeric(15,0) ,
    creation_date date ,
    created_by numeric(15,0) ,
    last_update_login numeric(10,0) ,
    description character varying(240) ,
    source_lang character varying(4) ,
    flex_value_meaning character varying(150) ,
    legalhold character varying(5) ,
    legaldescription character varying(255) ,
    dataowner character varying(255) ,
    purgedate date 
)
WITH (appendonly=true, orientation=column) DISTRIBUTED BY (flex_value_id);


set allow_system_table_mods="DML";
UPDATE pg_class
SET
	relpages = 1::int,
	reltuples = 0.0::real
WHERE relname = 'outer_table' AND relnamespace = (SELECT oid FROM pg_namespace WHERE nspname = 'ofa_applsys');
UPDATE pg_class
SET
	relpages = 64::int,
	reltuples = 61988.0::real
WHERE relname = 'outer_table' AND relnamespace = (SELECT oid FROM pg_namespace WHERE nspname = 'applsys');

CREATE TABLE inner_table (
    flex_value_set_id numeric(10,0) ,
    flex_value_id numeric(15,0) ,
    flex_value character varying(150) ,
    last_update_date date ,
    last_updated_by numeric(15,0) ,
    creation_date date ,
    created_by numeric(15,0) ,
    last_update_login numeric(15,0) ,
    enabled_flag character varying(1) ,
    summary_flag character varying(1) ,
    start_date_active date ,
    end_date_active date ,
    parent_flex_value_low character varying(60) ,
    parent_flex_value_high character varying(60) ,
    structured_hierarchy_level numeric(15,0) ,
    hierarchy_level character varying(30) ,
    compiled_value_attributes character varying(2000) ,
    value_category character varying(30) ,
    attribute1 character varying(240) ,
    attribute2 character varying(240) ,
    attribute3 character varying(240) ,
    attribute4 character varying(240) ,
    attribute5 character varying(240) ,
    attribute6 character varying(240) ,
    attribute7 character varying(240) ,
    attribute8 character varying(240) ,
    attribute9 character varying(240) ,
    attribute10 character varying(240) ,
    attribute11 character varying(240) ,
    attribute12 character varying(240) ,
    attribute13 character varying(240) ,
    attribute14 character varying(240) ,
    attribute15 character varying(240) ,
    attribute16 character varying(240) ,
    attribute17 character varying(240) ,
    attribute18 character varying(240) ,
    attribute19 character varying(240) ,
    attribute20 character varying(240) ,
    attribute21 character varying(240) ,
    attribute22 character varying(240) ,
    attribute23 character varying(240) ,
    attribute24 character varying(240) ,
    attribute25 character varying(240) ,
    attribute26 character varying(240) ,
    attribute27 character varying(240) ,
    attribute28 character varying(240) ,
    attribute29 character varying(240) ,
    attribute30 character varying(240) ,
    attribute31 character varying(240) ,
    attribute32 character varying(240) ,
    attribute33 character varying(240) ,
    attribute34 character varying(240) ,
    attribute35 character varying(240) ,
    attribute36 character varying(240) ,
    attribute37 character varying(240) ,
    attribute38 character varying(240) ,
    attribute39 character varying(240) ,
    attribute40 character varying(240) ,
    attribute41 character varying(240) ,
    attribute42 character varying(240) ,
    attribute43 character varying(240) ,
    attribute44 character varying(240) ,
    attribute45 character varying(240) ,
    attribute46 character varying(240) ,
    attribute47 character varying(240) ,
    attribute48 character varying(240) ,
    attribute49 character varying(240) ,
    attribute50 character varying(240) ,
    attribute_sort_order numeric(15,0) ,
    legalhold character varying(5) ,
    legaldescription character varying(255) ,
    dataowner character varying(255) ,
    purgedate date 
)
WITH (appendonly=true, orientation=column) DISTRIBUTED BY (flex_value_set_id ,flex_value_id);

CREATE INDEX inner_table_idx ON inner_table USING btree (flex_value_id);
UPDATE pg_class
SET
	relpages = 64::int,
	reltuples = 30994.0::real
WHERE relname = 'inner_table' AND relnamespace = (SELECT oid FROM pg_namespace WHERE nspname = 'applsys');


insert into outer_table values(12345,'ZHS');
insert into outer_table values(012345,'ZHS');
insert into inner_table values(54321,12345);
insert into inner_table values(654321,123456);

CREATE TABLE foo(fid numeric(10,0));
insert into foo VALUES(54321);

set optimizer=on;
set optimizer_segments=64;
set optimizer_enable_bitmapscan=on; 
select disable_xform('CXformInnerJoin2HashJoin');
select disable_xform('CXformInnerJoin2IndexGetApply');

SELECT fid FROM
(SELECT b1.flex_value_set_id, b1.flex_value_id
FROM
inner_table b1,
outer_table c1
WHERE
b1.flex_value_id = c1.flex_value_id 
and c1.language = 'ZHS') bar,
foo
WHERE foo.fid = bar.flex_value_set_id;
