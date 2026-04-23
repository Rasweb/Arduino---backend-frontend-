CREATE TABLE "tests" (
	"test_id" text PRIMARY KEY NOT NULL,
	"device_id" text NOT NULL,
	"lineNumber" integer,
	"prodname" text,
	"serialnr" text,
	"teststatus" text,
	"data" jsonb NOT NULL,
	"freeText" text,
	"timestamp" timestamp NOT NULL
);
