#!/usr/bin/env sysbench
-- This test is designed stressing memory access while minimising CPU usage
--
-- Adjust parameters to make this a memory constrainted test
--

require("oltp_common")

-- Add string_size to the list of standard OLTP options
sysbench.cmdline.options.string_size =
   {"Size on the string data", 767}

function create_table(drv, con, table_num)

   con:query( string.format([[
CREATE TABLE IF NOT EXISTS sbtest%d(
  id INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
  b VARBINARY(767) NOT NULL,
  PRIMARY KEY (id)
)]], table_num))


   if (sysbench.opt.table_size > 0) then
      print(string.format("Inserting %d records into 'sbtest%d'",
                          sysbench.opt.table_size, table_num))
   end

   con:bulk_insert_init("INSERT INTO sbtest" .. table_num .. "(b) VALUES ")
   for i = 1, sysbench.opt.table_size do
     local b = sysbench.rand.string('@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@')
     -- local b = sysbench.rand.varstring(31,31)
     -- con:bulk_insert_next("(\"" .. b .. "\")")
     con:bulk_insert_next("(CONCAT(REPEAT(_binary\"a long tale about war and peace.\", " .. (sysbench.opt.string_size - 31) / 32 .. "), _binary\"" .. b .. "\"))\n")
   end

   con:bulk_insert_done()
   con:query("SELECT SUM(LENGTH(b)) FROM sbtest" .. table_num)
end

function prepare_statements()
   stmt_defs['point_selects'][1] = "SELECT 1 FROM sbtest%u WHERE id = ? AND RIGHT(b, 16) = _binary\"a long tale abou\""
   prepare_point_selects()
end

function cleanup()
   oltp_cleanup()
end

function event()
   execute_point_selects()
end
