import { createContext, useContext, useEffect, useRef, useState, type ReactNode } from "react";
import {flecs} from "../../flecs.js";
import { FlecsAsync } from "./flecsAsync.ts"


type FlecsConnection = any | null; // ReturnType<typeof flecs.connect>

export interface FlecsConnectionState {
  connection: FlecsAsync | null;
  status: string;
  heartbeat: any | null;
}

const defaultState: FlecsConnectionState = {
  connection: null,
  status: "Disconnected",
  heartbeat: null,
};

export const FlecsConnectionContext = createContext<FlecsConnectionState>(defaultState);

interface FlecsProviderProps {
  host?: string;
  pollIntervalMs?: number;
  onFallback?: () => void;
}

export const FlecsConnectionProvider = ({
  children,
  host = "localhost",
  pollIntervalMs = 1000,
  onFallback,
}: FlecsProviderProps & { children: ReactNode }) => {
  const [status, setStatus] = useState<string>("Disconnected");
  const [heartbeat, setHeartbeat] = useState<any | null>(null);
  const connectionRef = useRef<FlecsConnection>(null);

  useEffect(() => {
    
    
    // Initialize persistent connection
    connectionRef.current = new FlecsAsync({
      host,
      poll_interval_ms: pollIntervalMs,

      on_fallback: onFallback,

      // on_status: (s) => {
      //   setStatus(s.toString()); // convert enum to string
      // },
      
      on_status: (s : any) => {
        setStatus(flecs.ConnectionStatus.toString(s)); // convert enum to string
      },

      on_heartbeat: (msg : any) => {
        setHeartbeat(msg);
      },

      on_host: (h : any) => {
        console.log("Connected to host:", h);
      },
    });

    // Cleanup on unmount
    return () => {
      if (connectionRef.current) {
        connectionRef.current.disconnect();
        connectionRef.current = null;
      }
    };

    // return () => {
    //   connectionRef.current?.close?.();
    //   connectionRef.current = null;
    // };
  }, [host, pollIntervalMs, onFallback]);

  return (
    <FlecsConnectionContext.Provider
      value={{
        connection: connectionRef.current,
        status,
        heartbeat,
      }}
    >
      {children}
    </FlecsConnectionContext.Provider>
  );
};

///export default FlecsConnectionState;
