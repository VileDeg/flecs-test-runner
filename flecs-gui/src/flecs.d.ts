// flecs.d.ts

export interface ConnectionParams {
  host?: string;
  timeout_ms?: number;
  poll_interval_ms?: number;
  retry_interval_ms?: number;
  max_retry_count?: number;
  fallback_host?: string;
  on_fallback?: () => void;
  on_status?: (status: symbol) => void;
  on_host?: (host: string) => void;
  on_heartbeat?: (msg: any) => void;
}

export interface RequestParams {
  poll_interval_ms?: number;
  managed?: boolean;
  persist?: boolean;
  dryrun?: boolean;
  [key: string]: any;
}

export interface Request {
  status: symbol;
  do(cache?: boolean): void;
  redo(): void;
  resume(): void;
  cancel(): void;
  abort(keepPersist?: boolean): void;
  now(): void;
}

export interface Connection {
  status: symbol;
  mode: symbol;
  worldInfo?: any;
  requests: {
    sent: number;
    received: number;
    error: number;
  };
  bytes: {
    received: number;
  };
  
  connect(host: string): void;
  disconnect(): void;
  set_managed_params(params: { poll_interval_ms?: number }): void;
  request_managed(): void;
  
  entity(
    path: string,
    params: RequestParams,
    recv: (msg: any) => void,
    err?: (msg: any) => void,
    abort?: (req: Request) => void
  ): Request;
  
  query(
    query: string,
    params: RequestParams,
    recv: (msg: any) => void,
    err?: (msg: any) => void,
    abort?: (req: Request) => void
  ): Request;
  
  queryName(
    query: string,
    params: RequestParams,
    recv: (msg: any) => void,
    err?: (msg: any) => void,
    abort?: (req: Request) => void
  ): Request;
  
  set(path: string, component: string, value: any): Request;
  get(path: string, params: RequestParams, recv: (msg: any) => void, err?: (msg: any) => void): Request;
  add(path: string, component: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  remove(path: string, component: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  enable(path: string, component: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  disable(path: string, component: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  create(path: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  delete(path: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  world(recv: (msg: any) => void, err?: (msg: any) => void): Request;
  scriptUpdate(path: string, code: string, params: RequestParams, recv: (msg: any) => void, err?: (msg: any) => void): Request;
  action(action: string, recv?: (msg: any) => void, err?: (msg: any) => void): Request;
  request(path: string, params: RequestParams, recv?: (msg: any) => void, err?: (msg: any) => void, abort?: (req: Request) => void): Request;
}

export declare const flecs: {
  ConnectionStatus: {
    Connecting: symbol;
    RetryConnecting: symbol;
    Connected: symbol;
    Disconnected: symbol;
    toString(value: symbol): string;
  };
  
  ConnectionMode: {
    Unknown: symbol;
    Remote: symbol;
    Wasm: symbol;
    toString(value: symbol): string;
  };
  
  RequestStatus: {
    Pending: symbol;
    Alive: symbol;
    Done: symbol;
    Aborting: symbol;
    Aborted: symbol;
    Failed: symbol;
  };
  
  trimQuery(query: string): string;
  connect(params: ConnectionParams): Connection;
  
  captureKeyboardEvents?: (capture: boolean) => void;
  has3DCanvas?: boolean;
};

export default flecs;