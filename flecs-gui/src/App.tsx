import { useState } from 'react'

import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'

import { FlecsConnectionProvider } from "./context/flecsConnection/flecsConnectionProvider.tsx";

import {LandingPage} from './components/landingPage/landingPage.tsx'
import {ResultsPage} from './components/resultsPage/resultsPage.tsx'

//import flecs from '../flecs.js'

import './App.css'  

import { Container, Title, Subtitle, Button, Output } from './styles'


// Default port 27750

// Main App Component
const App = () => {
  const [entities, setEntities] = useState(null);
  const [components, setComponents] = useState(null);
  const [responseMessage, setResponseMessage] = useState("");

  const [testFile, setTestFile] = useState<File | null>(null);
  
  const [testMode, setTestMode] = useState(false);
  const [testsUploaded, setTestsUploaded] = useState(false);
  
  const onUpload = (file: File) => {
    setTestFile(file);
  };
  
  // Comes from LandingPage
  const onTestsUploaded = () => {
    setTestsUploaded(true);
  };

  // Fetch entities from the Flecs REST API
  const fetchEntities = async () => {
    try {
      
      //let conn = flecs.connect("localhost");
      // let conn = flecs.connect({
      //   host: "localhost"
      // });
      //console.log("REACEHD HERE");
      //const response = conn.entity(`Sun`);
      const response = await fetch(`${BASE_URL}/entity/Sun`);
      if (!response.ok) {
        throw new Error(`Failed to fetch entities: ${response.statusText}`);
      }
      const data = await response.json();
      setEntities(data);
    } catch (error) {
      //setEntities(null);
      setEntities({ error: error.message });
      //console.error("Error fetch entities");
    }
  };
  
  // Create an entity named UnitTestN and add a UnitTest component
  const createUnitTestEntity = async () => {
    const entityName = `UnitTest0`; 
    const systemName = "SystemToRun";

    const entityData = {
      path: entityName,
      components: {
        UnitTest: {
          system_name: systemName, // Add the system name to the UnitTest component
        },
      },
    };

    // Create entity
    try {
      //let conn = flecs.connect("localhost");
      
      
      const response = await fetch(`${BASE_URL}/entity/${entityName}`, {
        method: "PUT",
        // headers: {
        //   "Content-Type": "application/json",
        // },
        //body: JSON.stringify(entityData),
      });

      if (!response.ok) {
        throw new Error(`Failed to create entity: ${response.statusText}`);
      }

      const data = await response.json();
      console.log("Entity created:", data);
      
      
      setResponseMessage(`Entity ${entityName} created successfully!`);
      
    } catch (error) {
      setResponseMessage(`Error creating entity: ${error instanceof Error ? error.message : "Unknown error"}`);
      console.error("Error creating entity:", error);
    }
    
    // Add component
    try {
      const url = new URL(`${BASE_URL}/component/${entityName}?`);
      url.searchParams.set("component", "test_runner..UnitTest");
      url.searchParams.set("value", "{\"systemName\":\"testSystem\"}");

      
      const response = await fetch(url.toString(), {
        method: "PUT",
        // headers: {
        //   "Content-Type": "application/json",
        // },
        //body: JSON.stringify(entityData),
      });

      if (!response.ok) {
        throw new Error(`Failed to create component: ${response.statusText}`);
      }

      const data = await response.json();
      console.log("Test component added:", data);
      
      
      setResponseMessage(`Component for ${entityName} created successfully!`);
      
    } catch (error) {
      setResponseMessage(`Error creating component: ${error instanceof Error ? error.message : "Unknown error"}`);
      console.error("Error creating component:", error);
    }
  };

  return (
    <FlecsConnectionProvider>
      <div style={{ display: "block" }}>
      {testMode ? (
        <Container>
          <Title>Flecs REST API GUI</Title>
          <Button onClick={fetchEntities}>Fetch Entities</Button>
          <Button onClick={createUnitTestEntity}>Create Unit Test Entity</Button>

          <Subtitle>Entities</Subtitle>
          <Output>{entities ? JSON.stringify(entities, null, 2) : "Click 'Fetch Entities' to load data..."}</Output>

          <Subtitle>Response</Subtitle>
          <Output>{responseMessage || "Click 'Create Unit Test Entity' to create an entity..."}</Output>
        </Container>
      ): (
        
        <div style={{ marginTop: "60px" }}>
          
          {testsUploaded == false ? (
            <LandingPage onTestsUploaded={onTestsUploaded} />
          ) : (
            <ResultsPage />
          )}
        </div>
        )    
      }
      </div>
    </FlecsConnectionProvider>
  );
}

export default App;

// function App() {
//   const [count, setCount] = useState(0)

//   return (
//     <>
//       <div>
//         <a href="https://vite.dev" target="_blank">
//           <img src={viteLogo} className="logo" alt="Vite logo" />
//         </a>
//         <a href="https://react.dev" target="_blank">
//           <img src={reactLogo} className="logo react" alt="React logo" />
//         </a>
//       </div>
//       <h1>Vite + React</h1>
//       <div className="card">
//         <button onClick={() => setCount((count) => count + 1)}>
//           count is {count}
//         </button>
//         <p>
//           Edit <code>src/App.tsx</code> and save to test HMR
//         </p>
//       </div>
//       <p className="read-the-docs">
//         Click on the Vite and React logos to learn more
//       </p>
//     </>
//   )
// }

// export default App
