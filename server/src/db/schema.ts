
import { jsonb, pgTable, text, integer, timestamp } from "drizzle-orm/pg-core";

export const tests = pgTable("tests", {
  test_id: text("test_id").primaryKey(),
  device_id: text("device_id").notNull(),
  lineNumber: integer("lineNumber"),
  prodname: text("prodname"),
  serialnr: text("serialnr"),
  teststatus: text("teststatus"),
  data: jsonb("data"),
  freeText: text("freeText"),
  timestamp: timestamp("timestamp", { mode: "string" }).notNull(),
});




