# ==============================================================================
# Usage: sudo mysql -p -v --show-warnings < gdp-ribd.sql
# ==============================================================================
#
#	----- BEGIN LICENSE BLOCK -----
#	Applications for the Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2018-2019, Regents of the University of California.
#	All rights reserved.
#
#	Permission is hereby granted, without written agreement and without
#	license or royalty fees, to use, copy, modify, and distribute this
#	software and its documentation for any purpose, provided that the above
#	copyright notice and the following two paragraphs appear in all copies
#	of this software.
#
#	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
#	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
#	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
#	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
#	OR MODIFICATIONS.
#	----- END LICENSE BLOCK -----

# install oqgraph engine (provided by mariadb-plugin-oqgraph package)
install soname 'ha_oqgraph';
show engines;

#
# front-end daemon expects this specific database name
#
create database if not exists gdp_rib;
show databases;

#
# gdp-ribd expects this database user, identified by '<password>'
# (gdp-ribd is co-located, database is only accessible from localhost)
#
# PRODUCTION NOTE: change 'gd_rib_pass' here and in the gdp-ribd source
# to some suitable secret ahead of build and installation.
#
GRANT ALL PRIVILEGES ON gdp_rib.*
	TO 'gdp_rib_user'@'127.0.0.1'
	IDENTIFIED BY 'gdp_rib_pass'
	WITH GRANT OPTION;
SELECT user, host FROM mysql.user;

# GRANT ALL PRIVILEGES will automatically create the user if not exists, as if
# CREATE USER 'gdp_rib_user'@'127.0.0.1';

# ==============================================================================
# load tables and stored procedures into gdp_rib database
# ==============================================================================
use gdp_rib;

# ==============================================================================
# tables
# ==============================================================================

#
# graph and nhops are entangled, so drop any prior table definitions as a unit
#
drop table if exists gdp_rib.graph;
drop table if exists gdp_rib.nhops;
drop table if exists gdp_rib.guids;

#
# gdp_rib.guids holds guid<->id assignments (oqgraph supports bigint id)
#
# guid is a unique gdp address (but multiple instances can exist in the graph)
# id is a unique oqgraph-friendly bigint which has been assigned to this guid
# ts is stamped whenever a nhop entry is withdrawn for this id (aka guid)
#
# guids expire after reaching zero nhops entries and ts < expiration
#
create table gdp_rib.guids
(
	guid BINARY(32) not null,
	id bigint unsigned not null auto_increment,
	ts TIMESTAMP,
	primary key (guid),
	key (id)
);

#
# gdp_rib.nhop holds directional graph arrows as id pairs (origid -> destid)
#
# origid is the origin id through which destid can be reached
# destid is the destination id reachable through originid
#
# nhops do not expire (as of 20190625!) but are withdrawn when links drop||close
#
create table gdp_rib.nhops
(
	origid bigint unsigned not null,
	destid bigint unsigned not null,
	primary key (origid, destid),
	key (destid)
);

#
# gdp_rib.graph is the graph engine viewport into gdp_rib.nhops state
#
# table format is specified by oqgraph engine (documented online)
#
# oqgraph engine maintains an in-memory graph and provides dijkstra algo service
#
create table gdp_rib.graph
(
	latch varchar(32) null,
	origid bigint unsigned null,
	destid bigint unsigned null,
	weight double null,
	seq bigint unsigned null,
	linkid bigint unsigned null,
	key (latch, origid, destid) using hash,
	key (latch, destid, origid) using hash
) ENGINE=OQGRAPH data_table='nhops' origid='origid' destid='destid';

# ==============================================================================
# stored procedures
# ==============================================================================

#
# display guids table with guid converted into base64 encoding (~= gdpname)
#
drop procedure if exists gdp_rib.select_all_guids;
delimiter //
create procedure gdp_rib.select_all_guids
(
)
begin
	select TO_BASE64(guid), id, ts from gdp_rib.guids order by ts;
end //
delimiter ;

#
# gdp_rib.drop_expired after reaching zero nhops entries and ts < expiration
#
# executed periodically to drop all qualifying (i.e. expired) guids entries
#
drop procedure if exists gdp_rib.drop_expired;
delimiter //
create procedure gdp_rib.drop_expired
(
)
begin
	#
	# expire stale guids (no nhops table references and ts expired)
	#
	delete from gdp_rib.guids where id not in
	 (select origid from gdp_rib.nhops union select destid from gdp_rib.nhops)
	 and ts < DATE_SUB(NOW(), INTERVAL 1 MINUTE);
end //
delimiter ;

#
# gdp_rib.add_nhop inserts a next hop arrow from eguid to dguid
#
# eguid is the next hop id through which dguid can be reached
# dguid is the destination id reachable through eguid
#
drop procedure if exists gdp_rib.add_nhop;
delimiter //
create procedure gdp_rib.add_nhop
(
	IN eguid BINARY(32),
	IN dguid BINARY(32)
)
begin
	set @eid = NULL;
	set @did = NULL;
	#
	# assign eguid,dguid ids (insert ignore avoids renumbering id if row exists)
	#
	insert ignore into gdp_rib.guids (guid, ts) values
	 (eguid, CURRENT_TIMESTAMP); # note ok that ts is not updated if row exists
	select id into @eid from gdp_rib.guids where guid = eguid;
	insert ignore into gdp_rib.guids (guid, ts) values
	 (dguid, CURRENT_TIMESTAMP); # note ok that ts is not updated if row exists
	select id into @did from gdp_rib.guids where guid = dguid;
	#
	# add next hop arrow if not already present
	#
	insert ignore into gdp_rib.nhops (origid, destid) values (@eid, @did);
	end //
delimiter ;

#
# gdp_rib.find_nhop returns nexthop from eguid toward closest dguid instance
#
drop procedure if exists gdp_rib.find_nhop;
delimiter //
create procedure gdp_rib.find_nhop
(
	IN eguid BINARY(32),
	IN dguid BINARY(32)
)
begin
	#
	# find ids representing eguid (aka oguid) and dguid
	#
	set @oid = NULL;
	set @did = NULL;
	select id into @oid from gdp_rib.guids where guid = eguid;
	select id into @did from gdp_rib.guids where guid = dguid;
	#
	# find next hop
	#
	select guid from graph inner join guids where latch='dijkstras' and 
	 origid = @oid and destid = @did and
	 weight = 1 and seq = 1 and id = linkid limit 1;
	#
end //
delimiter ;

#
# gdp_rib.mfind_nhop returns nexthop(s) from eguid toward many dguid instances
#
drop procedure if exists gdp_rib.mfind_nhop;
delimiter //
create procedure gdp_rib.mfind_nhop
(
	IN eguid BINARY(32),
	IN dguid BINARY(32)
)
begin
	#
	# find ids representing eguid (aka oguid) and dguid
	#
	set @oid = NULL;
	set @did = NULL;
	select id into @oid from gdp_rib.guids where guid = eguid;
	select id into @did from gdp_rib.guids where guid = dguid;
	#
	# find next hop(s) ordered by locality
	#
	select guid from gdp_rib.graph
	 lasthop inner join gdp_rib.graph nexthop inner join guids where
	  lasthop.latch='dijkstras' and lasthop.destid = @did and
	   lasthop.weight = 1 and
	  nexthop.latch='dijkstras' and nexthop.origid = @oid and
	   nexthop.destid = lasthop.linkid and nexthop.weight = 1 and
	   nexthop.seq = 1 and @oid != @did and guids.id = nexthop.linkid;
	#
end //
delimiter ;

#
# gdp_rib.delete_nhop deletes a nexthop arrow from eguid to dguid, if any
#
drop procedure if exists gdp_rib.delete_nhop;
delimiter //
create procedure gdp_rib.delete_nhop
(
	IN eguid BINARY(32),
	IN dguid BINARY(32)
)
begin
	set @eid = NULL;
	set @did = NULL;
	#
	# find ids representing eguid and dguid
	#
	select id into @did from gdp_rib.guids where guid = dguid;
	select id into @eid from gdp_rib.guids where guid = eguid;
	#
	# delete nhop entry (@eid, @did)
	#
	delete from gdp_rib.nhops where origid = @eid and destid = @did;
	#
	# update guids table ts field for dguid affected by this delete
	#
	update ignore gdp_rib.guids set ts=CURRENT_TIMESTAMP where id = @did;
	#
	# unrelated to above work, expire stale guids (see gdp_ribs.drop_expired)
	#
	delete from gdp_rib.guids where id not in
	 (select origid from gdp_rib.nhops union select destid from gdp_rib.nhops)
	 and ts < DATE_SUB(NOW(), INTERVAL 1 MINUTE);
	end //
delimiter ;

#
# gdp_rib.flush_nhops deletes all nexthop arrows from eguid to any guid, if any
#
drop procedure if exists gdp_rib.flush_nhops;
delimiter //
create procedure gdp_rib.flush_nhops
(
	IN eguid BINARY(32)
)
begin
	set @eid = NULL;
	#
	# find id representing eguid
	#
	select id into @eid from gdp_rib.guids where guid = eguid;
	#
	# update guids table ts field on all dguids affected by eguid flush
	#
    update ignore gdp_rib.guids set ts=CURRENT_TIMESTAMP where id in
	 (select destid from gdp_rib.nhops where origid = @eid);
	#
	# delete all nhop entries advertised by eguid
	#
	delete from gdp_rib.nhops where origid = @eid;
	#
	# unrelated to above work, expire stale guids (see gdp_ribs.drop_expired)
	#
	delete from gdp_rib.guids where id not in
	 (select origid from gdp_rib.nhops union select destid from gdp_rib.nhops)
	 and ts < DATE_SUB(NOW(), INTERVAL 1 MINUTE);
	end //
delimiter ;

# ==============================================================================
# unit tests (see ./test/README.md for additional tests)
# ==============================================================================

delete from guids;
delete from nhops;

call add_nhop(x'A1', x'A2');
call add_nhop(x'A2', x'A3');
call add_nhop(x'A3', x'A4');
call add_nhop(x'A4', x'A5');
call add_nhop(x'A5', x'A6');

call add_nhop(x'A7', x'A8');
call add_nhop(x'A8', x'A9');
call add_nhop(x'A9', x'B1');
call add_nhop(x'A9', x'B2');
call add_nhop(x'A9', x'B3');

# add shortcut from A2 directly to A5
call add_nhop(x'A2', x'A5');

# add path from A6 to A2 via A5
call add_nhop(x'A6', x'A5');
call add_nhop(x'A5', x'A2');

select * from nhops;
select HEX(guid),guid,id from guids;

call find_nhop(x'A1', x'A6');  # answer is HEX(guid) x'A2' aka \242

call find_nhop(x'A1', x'A6');  # answer is HEX(guid) x'A2' aka \242

call find_nhop(x'A2', x'A6');  # answer is HEX(GUID) x'A5' aka \245

call find_nhop(x'A6', x'A2');  # answer is HEX(GUID) x'A5' aka \245

call find_nhop(x'A1', x'A8');  # answer is the Empty set

call delete_nhop(x'A2', x'A5');
call find_nhop(x'A2', x'A6');  # answer has changed to HEX(GUID) x'A3' aka \243

call find_nhop(x'A7', x'B1');  # answer is HEX(GUID) x'A8' aka \250
call flush_nhops(x'A9');
call find_nhop(x'A7', x'B1');  # answer is the Empty set
call find_nhop(x'A7', x'A9');  # answer is HEX(GUID) x'A8' aka \250

# ==============================================================================
# test cleanup
# ==============================================================================
delete from guids;
delete from nhops;

# ==============================================================================
