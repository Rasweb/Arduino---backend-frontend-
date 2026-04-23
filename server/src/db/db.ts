/* Info
- https://expressjs.com/en/guide/database-integration.html#postgresql
- https://orm.drizzle.team/docs/get-started-postgresql

- https://orm.drizzle.team/docs/kit-overview
  - npx drizzle-kit generate - generate sql migration files based on schema
  - npx drizzle-kit migrate - apply migration files to database
  - npx drizzle-kit pull - pull database schema, convert to drizzle schema and save it
  - npx drizzle-kit push - push drizzle schema to database (declaration/schema changes)
  - npx drizzle-kit studio - connect to database and show proxy server for database browsing
  - npx drizzle-kit check - check for errors

  For future implementation: https://orm.drizzle.team/docs/rls
*/
import { drizzle } from "drizzle-orm/node-postgres";
import { Pool } from "pg";
import dotenv from "dotenv"
import * as schema from "./schema.js"
import { envSchema, zodFormat } from "../data/zodUtils.js";
import { ZodError }from "zod";
dotenv.config();


let validatedUrl;

try {
  validatedUrl = envSchema.parse(process.env.DATABASE_URL);
} catch (error) {
  if (error instanceof ZodError) {
    console.error("Env must match this: DATABASE_URL='postgresql://<username>:<password>@<hostname>/<database>'");
    const formattedErrors = zodFormat(error);
    throw { status: 400, message: "Bad Request: " + formattedErrors };
  } else {
    throw { status: 500, message: "Internal Server Error" };
  }
}

const pool = new Pool({
    connectionString: validatedUrl,

});

export const db = drizzle(pool, {schema});