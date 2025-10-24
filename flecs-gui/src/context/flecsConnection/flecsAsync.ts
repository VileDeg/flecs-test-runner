/* eslint-disable @typescript-eslint/no-explicit-any  */
/* esling-disable @typescript-eslint/ban-ts-comment */

// @ts-ignore TS7019
// @ts-ignore TS7006
// @ts-nocheck



import {flecs} from "../../flecs.js";

export class FlecsAsync {
  connection;
  isConnected : boolean = false;
  
  constructor(params: any) {
    this.connection = flecs.connect(params);
    this.isConnected = false;
  }

  // Wait for connection to be established
  async waitForConnection(timeoutMs = 5000) {
    return new Promise<void>((resolve, reject) => {
      if (this.isConnected) {
        resolve();
        return;
      }

      const timeout = setTimeout(() => {
        reject(new Error('Connection timeout'));
      }, timeoutMs);

      const originalOnStatus = this.connection.params.on_status;
      this.connection.params.on_status = (status) => {
        if (originalOnStatus) originalOnStatus(status);
        
        if (status === flecs.ConnectionStatus.Connected) {
          clearTimeout(timeout);
          this.isConnected = true;
          resolve();
        }
      };
    });
  }

  // Generic method wrapper
  _wrapMethod(methodName : string, ...args) {
    return new Promise<any>((resolve, reject) => {
      const method = (this.connection as any)[methodName];
      if (!method) {
        reject(new Error(`Method ${methodName} not found`));
        return;
      }
      
      let newArgs: any[];
      //if(methodName != "set") {
        // TODO: other methods without recv, err
        // Add success and error callbacks
      newArgs = [...args, resolve, reject];
      method.apply(this.connection, newArgs);
      // } else {
      //   newArgs = [...args];
      //   method.apply(this.connection, newArgs);
      //   resolve();
      // }
    });
  }

  // Entity operations
  async entity(path, params = {}) {
    return this._wrapMethod('entity', path, params);
  }

  async create(path) {
    return this._wrapMethod('create', path);
  }

  async delete(path) {
    return this._wrapMethod('delete', path);
  }

  // Component operations
  async add(path, component) {
    return this._wrapMethod('add', path, component);
  }

  async remove(path, component) {
    return this._wrapMethod('remove', path, component);
  }

   // Methods that DON'T take recv/err - return request objects
  async set(path, component, value) {
    // return new Promise((resolve, reject) => {
    //   try {
    //     const request = this.connection.set(path, component, value);
        
    //     // The request object handles the HTTP call internally
    //     // We need to hook into its completion somehow
    //     // Since it returns immediately, we assume success for now
    //     resolve(request);
    //   } catch (error) {
    //     reject(error);
    //   }
    // });
    return this._wrapMethod('set', path, component, value);
  }

  async get(path, params = {}) {
    return this._wrapMethod('get', path, params);
  }

  // async enable(path, component) {
  //   return this._wrapMethod('enable', path, component);
  // }

  // async disable(path, component) {
  //   return this._wrapMethod('disable', path, component);
  // }

  // Query operations
  async query(queryString, params = {}) {
    return this._wrapMethod('query', queryString, params);
  }

  // async queryName(queryName, params = {}) {
  //   return this._wrapMethod('queryName', queryName, params);
  // }

  // // World operations
  // async world() {
  //   return this._wrapMethod('world');
  // }

  // // Script operations
  // async scriptUpdate(path, code, params = {}) {
  //   return this._wrapMethod('scriptUpdate', path, code, params);
  // }

  // // Action operations
  // async action(action) {
  //   return this._wrapMethod('action', action);
  // }

  // // Generic request
  // async request(path, params = {}) {
  //   return this._wrapMethod('request', path, params);
  // }

  // Connection management
  disconnect() {
    this.connection.disconnect();
    this.isConnected = false;
  }

  // Getters for connection info
  get status() {
    return this.connection.status;
  }

  get worldInfo() {
    return this.connection.worldInfo;
  }
}

/* eslint-enable @typescript-eslint/no-explicit-any */