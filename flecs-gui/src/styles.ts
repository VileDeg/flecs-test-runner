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
