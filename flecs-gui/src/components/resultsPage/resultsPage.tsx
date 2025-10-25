// src/pages/ResultsPage.tsx
import React, { useEffect, useState } from "react";
import {
  Container,
  Header,
  Table,
  StatusBadge,
  ErrorBox,
  RefreshButton,
} from "./styles";

import {FLECS_PORT, BASE_URL, UNIT_TEST_COMPONENT_NAME} from "../../common/constants.ts"

// TODO: merge with types in Uploader
interface SystemInvocation {
  name: string;
  timesToRun: number;
}

interface UnitTest {
  name: string;
  systems: SystemInvocation[];
  scriptActual: string;
  scriptExpected: string;
  Executed?: { statusMessage: string };
  Passed?: object;
}

export const ResultsPage: React.FC = () => {
  const [tests, setTests] = useState<UnitTest[]>([]);
  const [errorMessage, setErrorMessage] = useState("");
  const [isPolling, setIsPolling] = useState(true);

  // Poll every 2 seconds
  useEffect(() => {
    let interval: NodeJS.Timeout;
    const fetchResults = async () => {
      try {
        const res = await fetch(`${BASE_URL}/entity?component=${UNIT_TEST_COMPONENT_NAME}`);
        if (!res.ok) throw new Error(`Server responded with ${res.status}`);
        const data = await res.json();

        // Flecs usually returns { results: [ { components... } ] }
        const entities = data.results ?? data;

        const parsedTests: UnitTest[] = entities.map((e: any) => {
          const ut = e.UnitTest as UnitTest;
          ut.Executed = e.Executed;
          ut.Passed = e.Passed;
          return ut;
        });

        setTests(parsedTests);
        setErrorMessage("");
      } catch (err: any) {
        setErrorMessage(`Error fetching results: ${err.message}`);
      }
    };

    if (isPolling) {
      fetchResults();
      interval = setInterval(fetchResults, 2000);
    }

    return () => clearInterval(interval);
  }, [isPolling]);

  const togglePolling = () => setIsPolling((prev) => !prev);

  return (
    <Container>
      <Header>Unit Test Results</Header>

      {errorMessage && <ErrorBox>{errorMessage}</ErrorBox>}

      <RefreshButton onClick={togglePolling}>
        {isPolling ? "⏸ Pause Updates" : "▶ Resume Updates"}
      </RefreshButton>

      <Table>
        <thead>
          <tr>
            <th>Test Name</th>
            <th>Status</th>
            <th>Status Message</th>
          </tr>
        </thead>
        <tbody>
          {tests.map((t) => (
            <tr key={t.name}>
              <td>{t.name}</td>
              <td>
                {t.Passed ? (
                  <StatusBadge status="passed">Passed</StatusBadge>
                ) : t.Executed ? (
                  <StatusBadge status="failed">Failed</StatusBadge>
                ) : (
                  <StatusBadge status="pending">Pending</StatusBadge>
                )}
              </td>
              <td>{t.Executed?.statusMessage ?? "-"}</td>
            </tr>
          ))}
        </tbody>
      </Table>
    </Container>
  );
};
