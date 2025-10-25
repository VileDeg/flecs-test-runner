// src/context/useFlecsConnection.ts
import { useContext } from "react";
import { FlecsConnectionContext, type FlecsConnectionState } from "./flecsConnectionProvider.tsx";

// Hook to access the shared connection
export const useFlecsConnection = (): FlecsConnectionState => {
  return useContext(FlecsConnectionContext);
};

export default useFlecsConnection;
