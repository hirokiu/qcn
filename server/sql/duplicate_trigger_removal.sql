/*



*/

use continual_archive;

create temporary table qcn_trigger_duplicate 
   (id int, hostid int, varietyid smallint, time_trigger double, result_name varchar(64));

create unique index qtdi on qcn_trigger_duplicate(id);

insert into qcn_trigger_duplicate
  (select min(id), hostid, varietyid, time_trigger, result_name
     from qcn_trigger
       group by hostid, varietyid, time_trigger, result_name);


select count(*) from qcn_trigger_duplicate;

delete from qcn_trigger where id not in (select id from qcn_trigger_duplicate);

drop table qcn_trigger_duplicate;

