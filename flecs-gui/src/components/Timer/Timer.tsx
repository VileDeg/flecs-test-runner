import { useState, useEffect } from "react";
import styled from "styled-components";

const TimerDisplay = styled.span`
  font-weight: normal;
  font-size: 1rem;
  color: #888;
  margin-left: 8px;
`;

interface TimerProps {
  startTime?: number; // timestamp when timer should start
  isRunning?: boolean; // whether timer should be running
}

export const Timer: React.FC<TimerProps> = ({ 
  startTime = Date.now(), 
  isRunning = true 
}) => {
  const [elapsedTime, setElapsedTime] = useState(0);

  useEffect(() => {
    if (!isRunning) return;

    const interval = setInterval(() => {
      const now = Date.now();
      const elapsed = Math.floor((now - startTime) / 1000); // elapsed time in seconds
      setElapsedTime(elapsed);
    }, 1000);

    return () => clearInterval(interval);
  }, [startTime, isRunning]);

  const formatTime = (seconds: number): string => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  };

  if (!isRunning) return null;

  return (
    <TimerDisplay>
      ({formatTime(elapsedTime)})
    </TimerDisplay>
  );
};