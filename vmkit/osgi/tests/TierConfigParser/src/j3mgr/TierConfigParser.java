package j3mgr;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class TierConfigParser
{
	String token_identifier;
	int token_integer;
	int lastChar;
	InputStream source;
	
	public void parse(String fileName) throws Throwable
	{
		InputStream is = new FileInputStream(fileName);
		parse(is);
	}
	
	public void parse(InputStream is) throws NoSuchElementException, IOException
	{
		source = is;
		lastChar = source.read();
		token_identifier = "";
		
		for (;;) {
			skipWhiteSpaceAndComments();
			if (lastChar == -1) break;
			
			if (!parseIdentifier()) {
				throw new NoSuchElementException(
					"'tier' or 'account' expected.");
			}
			
			if (parseTier()) {}
			else if (parseAccount()) {}
			else {
				if (lastChar == -1)
					break;
				
				throw new NoSuchElementException(
					"'tier' or 'account' expected.");
			}
		}
	}
	
	void skipWhiteSpaceAndComments() throws IOException
	{
		for (;;) {
			// Skip white space
			for (; (lastChar != -1) && Character.isWhitespace(lastChar);
				lastChar = source.read());
			
			if (lastChar != '#') break;
			
			// Skip a comment line
			lastChar = source.read();
			while (lastChar != '\n')
				lastChar = source.read();
			lastChar = source.read();
		}
	}
	
	boolean parseIdentifier() throws IOException
	{
		skipWhiteSpaceAndComments();
		
		if (!Character.isLetter(lastChar) && lastChar != '_') return false;
		
		StringBuilder sb = new StringBuilder();
		sb.appendCodePoint(lastChar);
		lastChar = source.read();
		
		while (Character.isLetterOrDigit(lastChar) || lastChar == '_') {
			sb.appendCodePoint(lastChar);
			lastChar = source.read();
		}
		
		token_identifier = sb.toString();
		return true;
	}

	boolean parseInteger() throws IOException
	{
		skipWhiteSpaceAndComments();
		
		StringBuilder sb = new StringBuilder();
		
		if (lastChar == '+' || lastChar == '-') {
			sb.appendCodePoint(lastChar);
			lastChar = source.read();
			
			if (!Character.isDigit(lastChar))
				throw new NoSuchElementException("Digit expected.");
		} else if (!Character.isDigit(lastChar))
			return false;
		
		do {
			sb.appendCodePoint(lastChar);
			lastChar = source.read();
		} while (Character.isDigit(lastChar));

		token_identifier = sb.toString();
		token_integer = Integer.parseInt(token_identifier);
		return true;
	}
	
	boolean parseTier() throws IOException
	{
		if (token_identifier.compareTo("tier") != 0) return false;
		
		if (!parseInteger())
			throw new NoSuchElementException("Integer expected.");
		
		skipWhiteSpaceAndComments();
		if (lastChar != '{')
			throw new NoSuchElementException("'{' expected.");
		lastChar = source.read();
		
		ArrayList<String> bundles = new ArrayList<String>();
		
		skipWhiteSpaceAndComments();
		if (lastChar != '}') {
			for (;;) {
				if (!parseIdentifier())
					throw new NoSuchElementException("bundle name expected.");
				
				bundles.add(token_identifier);
				
				skipWhiteSpaceAndComments();
				if (lastChar == '}') break;
				
				if (lastChar != ',')
					throw new NoSuchElementException("',' expected.");
				lastChar = source.read();
			}
		}
		
		if (lastChar != '}')
			throw new NoSuchElementException("'}' expected.");
		lastChar = source.read();
		
		return createTier(token_integer, bundles);
	}

	boolean parseAccount() throws IOException
	{
		if (token_identifier.compareTo("account") != 0) return false;

		skipWhiteSpaceAndComments();
		if (lastChar != '{')
			throw new NoSuchElementException("'{' expected.");
		lastChar = source.read();
		
		long callerTierID = -1;
		skipWhiteSpaceAndComments();
		if (lastChar != '*') {
			if (!parseInteger())
				throw new NoSuchElementException("Integer or '*' expected.");
			callerTierID = token_integer;
		} else
			lastChar = source.read();
		
		skipWhiteSpaceAndComments();
		if (lastChar != ',')
			throw new NoSuchElementException("',' expected.");
		lastChar = source.read();
		
		long calleeTierID = -1;
		skipWhiteSpaceAndComments();
		if (lastChar != '*') {
			if (!parseInteger())
				throw new NoSuchElementException("Integer or '*' expected.");
			calleeTierID = token_integer;
		} else
			lastChar = source.read();
		
		skipWhiteSpaceAndComments();
		if (lastChar != ',')
			throw new NoSuchElementException("',' expected.");
		lastChar = source.read();

		if (!parseIdentifier())
			throw new NoSuchElementException("'caller' or 'callee' expected.");
		
		boolean chargeCaller = (token_identifier.compareTo("caller") == 0);
		if (!chargeCaller && (token_identifier.compareTo("callee") != 0))
			throw new NoSuchElementException("'caller' or 'callee' expected.");
		
		skipWhiteSpaceAndComments();
		if (lastChar != '}')
			throw new NoSuchElementException("'}' expected.");
		lastChar = source.read();
		
		return createAccount(callerTierID, calleeTierID, chargeCaller);
	}
	
	boolean createTier(long tierID, List<String> bundleList)
	{
		System.out.print(tierID + "=");
		if (bundleList != null) {
			for (String bundleID : bundleList)
				System.out.print(bundleID + " ");
		}
		System.out.println();
		return true;
	}
	
	boolean createAccount(
		long callerTierID, long calleeTierID, boolean chargeCaller)
	{
		System.out.println(
			callerTierID + "=>" + calleeTierID + ":" +
			(chargeCaller ? "caller" : "callee"));
		return true;
	}
}
