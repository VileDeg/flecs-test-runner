import styled from "styled-components";

// Styled components
export const Container = styled.div`
  font-family: Arial, sans-serif;
  padding: 20px;
`;

export const Button = styled.button`
  padding: 10px 20px;
  margin: 10px;
  background-color: #007bff;
  color: white;
  border: none;
  border-radius: 5px;
  cursor: pointer;

  &:hover {
    background-color: #0056b3;
  }
`;

export const Output = styled.pre`
  background:rgb(0, 0, 0);
  padding: 10px;
  border: 1px solid #ddd;
  overflow-x: auto;
`;

export const Title = styled.h1`
  color: #333;
`;

export const Subtitle = styled.h2`
  color: #555;
`;

// TopBar components
export const TopBar = styled.div`
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  z-index: 1000;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 24px;
  background-color: #2d2d2d;
  border-bottom: 1px solid #404040;
  box-shadow: 0 2px 8px rgba(0,0,0,0.3);
`;

export const TopBarLeft = styled.div`
  display: flex;
  align-items: center;
  gap: 16px;
`;

export const TopBarRight = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
`;

export const AppTitle = styled.h1`
  margin: 0;
  font-size: 1.5rem;
  font-weight: 600;
  color: #e0e0e0;
`;

export const ConnectionBadge = styled.span<{ $status: string }>`
  padding: 4px 12px;
  border-radius: 16px;
  font-size: 0.875rem;
  font-weight: 500;
  background-color: ${({ $status }) => 
    $status === "Connected" ? "#d4edda" : 
    ($status === "Connecting" || $status === "RetryConnecting") ? "#fff3cd" : "#f8d7da"
  };
  color: ${({ $status }) => 
    $status === "Connected" ? "#155724" : 
    ($status === "Connecting" || $status === "RetryConnecting") ? "#856404" : "#721c24"
  };
`;

export const NavButton = styled.button<{ $active?: boolean }>`
  padding: 8px 16px;
  border: 1px solid #007bff;
  border-radius: 6px;
  background-color: ${({ $active }) => $active ? "#007bff" : "transparent"};
  color: ${({ $active }) => $active ? "white" : "#e0e0e0"};
  cursor: pointer;
  font-weight: 500;
  transition: all 0.2s;

  &:hover {
    background-color: ${({ $active }) => $active ? "#0056b3" : "rgba(255,255,255,0.1)"};
    border-color: ${({ $active }) => $active ? "#0056b3" : "#4da3ff"};
  }
`;

export const CenteredContent = styled.div`
  font-family: "Inter", sans-serif;
  padding: 24px;
  max-width: 800px;
  margin: 0 auto;
  text-align: center;
  min-height: 60vh;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
`;

export const MainContent = styled.div`
  font-family: "Inter", sans-serif;
  padding: 88px 24px 24px 24px; /* Top padding accounts for fixed TopBar height */
  max-width: 800px;
  margin: 0 auto;
`;
