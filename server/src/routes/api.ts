import express from "express";
const router = express.Router();
import {z, ZodError } from "zod";
import { zodFormat } from "../data/zodUtils.js";
import { db } from "../db/db.js";
import { tests } from "../db/schema.js";
import { and, gt, lt, eq } from 'drizzle-orm';

const DeviceIdSchema = z.object({
  device_id:z.string().min(1, "device_id is required"),
})

const DateSchema = z.object({
  start_time: z.string().min(1, "start date is required"),
  end_time: z.string().min(1, "end date is required"),
})

router.get("/tests", async (req,res) => {
  try {
    const result = await db.query.tests.findMany();
    res.json({
      success: true,
      count: result.length,
      data: result
    });
    
  } catch (error) {
      if(error instanceof ZodError){      
        const formattedErrors = zodFormat(error);
        return res.status(400).json({message:"Bad Request", error: formattedErrors});

      } else {
        return res.status(500).json({message: "Internal server error"});
      }
    }
})

router.get("/tests/filter-by-date", async (req,res) => {
  try {
    const {start_time, end_time} = DateSchema.parse(req.query);

    const startIso = new Date(start_time).toISOString();
    const endIso = new Date(end_time).toISOString();

    if(endIso < startIso) {
      res.json({
        success:false,
        message: "End date must be bigger or equal to start date",
      });
    };
    const results = await db.select().from(tests).where(
      and(
        // gt - greater than
        // lt - less than
        gt(tests.timestamp, startIso),
        lt(tests.timestamp, endIso)
      )
    );
    res.json({
      success: true,
      count: results.length,
      data: results
    });
  } catch (error) {
      if(error instanceof ZodError){
        const formattedErrors = zodFormat(error);
        return res.status(400).json({message: "Bad Request", error: formattedErrors});
      } else {
        return res.status(500).json({message: "Internal server error"});
      }
    }
});

router.get("/tests/:device_id", async (req, res) => {
  try {
    const { device_id } = DeviceIdSchema.parse(req.params);
    const results = await db.select().from(tests).where(eq(tests.device_id, device_id));

    if(results.length === 0){
      return res.status(404).json({
        success: false,
        error: `No tests found for device_id: ${device_id}`
      });
    }
    res.json({
      success: true,
      device_id: device_id,
      count: results.length,
      data: results
    });
  } catch (error) {
    if(error instanceof ZodError){      
      const formattedErrors = zodFormat(error);
      return res.status(400).json({message:"Bad Request", error: formattedErrors});

    } else {
      return res.status(500).json({message: "Internal server error"});
    }
  }
});

export default router;