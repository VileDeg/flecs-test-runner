import { useState } from 'react'

import { FlecsConnectionProvider } from "./context/flecsConnection/flecsConnectionProvider.tsx";

import {LandingPage} from './components/landingPage/landingPage.tsx'
import {ResultsPage} from './components/resultsPage/resultsPage.tsx'

import { useFlecsConnection } from "./context/flecsConnection/useFlecsConnection.ts";

import './App.css'  

import { 
  Container, 
  Title, 
  Subtitle, 
  Button, 
  Output, 
  TopBar, 
  TopBarLeft, 
  TopBarRight, 
  AppTitle, 
  ConnectionBadge, 
  NavButton, 
  CenteredContent, 
  MainContent 
} from './styles'
import { Header, StatusBar } from './components/landingPage/styles.ts'
import { FLECS_PORT, BASE_URL } from './common/constants.ts'

// Default port 27750

// Main App Component
export const App = () => {
  return (
    <FlecsConnectionProvider>
      <AppContent />
    </FlecsConnectionProvider>
  );
}

// App content with connection context
const AppContent = () => {
  const [entities, setEntities] = useState<any>(null);
  const [responseMessage, setResponseMessage] = useState("");
  
  const [testMode] = useState(false);
  const [currentPage, setCurrentPage] = useState<'landing' | 'results'>('landing');
  
  const { status } = useFlecsConnection();
  
  // Comes from LandingPage
  const onTestsUploaded = () => {
    setCurrentPage('results'); // Automatically switch to results page after tests are uploaded
  };

  // Navigation functions
  const goToLandingPage = () => {
    setCurrentPage('landing');
  };

  const goToResultsPage = () => {
    setCurrentPage('results');
  };

  // Render the top bar when connected
  const renderTopBar = () => (
    <TopBar>
      <TopBarLeft>
        <AppTitle>Unit Test Runner</AppTitle>
        <ConnectionBadge $status={status}>
          {status === "Connected" && "✅ Connected"}
          {(status === "Connecting" || status === "RetryConnecting") && "🔄 Connecting..."}
          {status === "Disconnected" && "❌ Disconnected"}
        </ConnectionBadge>
      </TopBarLeft>
      <TopBarRight>
        <NavButton 
          $active={currentPage === 'landing'} 
          onClick={goToLandingPage}
        >
          Upload Tests
        </NavButton>
        <NavButton 
          $active={currentPage === 'results'} 
          onClick={goToResultsPage}
        >
          View Results
        </NavButton>
      </TopBarRight>
    </TopBar>
  );

  // Render centered content when not connected
  const renderCenteredContent = () => (
    <CenteredContent>
      <Header>Unit Test Runner</Header>
      <StatusBar $status={status}>
        {(status === "Connecting" || status === "RetryConnecting") 
            && `Trying to connect to port ${FLECS_PORT}...`}
        {status === "Connected" && `✅ Connected to port ${FLECS_PORT}`}
        {status === "Disconnected" && "❌ Connection failed"}
      </StatusBar>
    </CenteredContent>
  );

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
      setEntities({ error: (error as Error).message });
      //console.error("Error fetch entities");
    }
  };
  
  // Create an entity named UnitTestN and add a UnitTest component
  const createUnitTestEntity = async () => {
    const entityName = `UnitTest0`;

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
        <div>
          {status === "Connected" ? (
            <>
              {renderTopBar()}
              <MainContent>
                {currentPage === 'landing' ? (
                  <LandingPage onTestsUploaded={onTestsUploaded} />
                ) : (
                  <ResultsPage />
                )}
              </MainContent>
            </>
          ) : (
            renderCenteredContent()
          )}
        </div>
        )    
      }
    </div>
  );
}



