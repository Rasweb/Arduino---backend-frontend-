import type { NextFunction, Request, Response } from 'express';

const errorHandler = (err: Error, req: Request, res: Response, next: NextFunction ) => {
    console.error(err.stack);
    return res.status(500).json({message: "Internal server error"});
};

// 404 handler
const notFoundHandler =  (req: Request, res: Response) => {
    return res.status(404).json({message: "Route not found"});
};

export {errorHandler, notFoundHandler};
