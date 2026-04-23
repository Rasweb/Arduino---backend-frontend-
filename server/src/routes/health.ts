import  express from "express";
const router = express.Router();
import { db } from "../db/db.js";


router.get("/", async (req, res) => {
  try {
    const results = await db.query.tests.findMany();
    res.json({
      success: true,
      status: "operational",
      timestamp: new Date().toISOString(),
      testsInMemory: results.length,
      uptime: process.uptime()
    });
  } catch (error) {
    return res.status(500).json({message: "Internal server error"});
  }
});

export default router;
